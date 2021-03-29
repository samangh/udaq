#include <bytes.h>
#include <udaq/devices/safibra/sigproc_server.h>

#include "tcp_client.h"
#include "tcp_data_offsets.h"
#include "tcp_header.h"

using namespace std::placeholders;

namespace udaq::devices::safibra {
/* Returns the position of the header sync if found, -1 otherwise*/
int find_sync(const std::vector<unsigned char> &data) {
    for (size_t i = 0; i < data.size() - 2; i++)
        if (data[i] == 0x55 && data[i + 1] == 0x00 && data[i] == 0x55)
            return i;

    return -1;
};

udaq::devices::safibra::SigprogServer::SigprogServer(
    on_error_cb_t on_error_cb, on_client_connected_cb_t on_client_connected_cb,
    on_client_disconnected_cb_t on_client_disconnected_cb,
    on_start_cb_t on_start, on_stop_cb_t on_stop,
    on_data_available_cb_t on_data_available_cb)
    : m_on_data_available_cb(on_data_available_cb),
      m_client(std::make_unique<safibra_tcp_client>(
          on_error_cb, on_client_connected_cb, on_client_disconnected_cb,
          on_start, on_stop,
          std::bind(&SigprogServer::on_data_available_cb_from_tcp, this, _1, _2))) {}

udaq::devices::safibra::SigprogServer::~SigprogServer() {}

void udaq::devices::safibra::SigprogServer::start(const int port) {
    m_client->start(port);
}

void udaq::devices::safibra::SigprogServer::stop() { m_client->stop(); }

bool udaq::devices::safibra::SigprogServer::is_running() {
    return m_client->is_running();
}

void udaq::devices::safibra::SigprogServer::on_data_available_cb_from_tcp(
    const uint8_t *buff, size_t length) {
    using namespace udaq::common::bytes;

    {
        const std::lock_guard lock(m_mutex);

        /* Copy data to our local bufer */
        m_stream_buffer.insert(m_stream_buffer.end(), buff, buff + length);

        /* Not enough data*/
        if (m_stream_buffer.size() < MINIMUM_TOTAL_SIZE)
            return;

        /* Find sync */
        auto start = find_sync(m_stream_buffer);
        if (start < 0)
            return;

        /* If we get data midway, delete stuff before sync */
        if (start > 0)
            m_stream_buffer.erase(m_stream_buffer.begin(),
                                  m_stream_buffer.begin() + start);

        int msg_pos = 0;
        while (true) {
            /* Check that we have the header */
            if (m_stream_buffer.size() - msg_pos < HEADER_LENGTH)
                break;
            Header header(m_stream_buffer, msg_pos);

            /* Check we have the whole message */
            if (m_stream_buffer.size() - msg_pos < header.message_size)
                break;

            /* Get sensor from our map of sensors */
            auto find = m_data_buffer.find(header.sensor_id);
            if (find == m_data_buffer.end()) {
                m_data_buffer.insert({header.sensor_id, SensorReadout()});
                find = m_data_buffer.find(header.sensor_id);
            }
            SensorReadout &readout = find->second;

            for (size_t i = 0; i < header.no_readouts; ++i) {
                auto seconds = to_uint64(&m_stream_buffer[msg_pos + DATA_POS + 24 * i]);
                auto milliseconds = to_uint64(&m_stream_buffer[msg_pos + DATA_POS + 24 * i + 8]);
                readout.time.push_back(seconds + 1E-6 * milliseconds);
                readout.readouts.push_back(to_double(&m_stream_buffer[msg_pos + DATA_POS + 24 * i + 2 * 8]));
            }

            msg_pos += header.message_size;
        }

        /* Remove things that we have used */
        m_stream_buffer.erase(m_stream_buffer.begin(),
                              m_stream_buffer.begin() + msg_pos);
    }

    if (m_on_data_available_cb != nullptr)    
        m_on_data_available_cb(get_data_buffer());
}

std::map<std::string, SensorReadout> udaq::devices::safibra::SigprogServer::get_data_buffer() {
    std::lock_guard lock(m_mutex);

//    auto buffer = std::move(m_data_buffer);
//    m_data_buffer = std::map<std::string, SensorReadout>();
//    return buffer;
    return std::move(m_data_buffer);
}

} // namespace udaq::devices::safibra
