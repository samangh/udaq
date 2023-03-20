#include <sg/bytes.h>
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
            return (int)i;

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
          std::bind(&SigprogServer::on_data_available_cb_from_tcp, this, _1))) {}

udaq::devices::safibra::SigprogServer::~SigprogServer() {}

void udaq::devices::safibra::SigprogServer::start(const int port) {
    m_client->start(port);
}

void udaq::devices::safibra::SigprogServer::stop() { m_client->stop(); }

int udaq::devices::safibra::SigprogServer::number_of_clients() const {
    return m_client->number_of_clients();
}

bool udaq::devices::safibra::SigprogServer::is_running() const {
    return m_client->is_running();
}

uint32_t udaq::devices::safibra::SigprogServer::compute_checksum(const unsigned char *buffer, size_t message_size) {
    uint32_t* p = (uint32_t*)(buffer);

    //Sum until the last byte of the header
    auto count = (message_size) / 4 - 1;

    uint32_t sum = 0;
    for (size_t i = 0; i < count; ++i)
        sum += p[i];

    return sum;
}

void udaq::devices::safibra::SigprogServer::on_data_available_cb_from_tcp(size_t length) {
    if (m_on_data_available_cb != nullptr)
        m_on_data_available_cb();
}

std::vector<SensorReadout> udaq::devices::safibra::SigprogServer::get_data_buffer() {
    using namespace sg::bytes;

    auto buff_in= m_client->get_buffer();
    m_stream_buffer.insert(m_stream_buffer.end(), buff_in.begin(), buff_in.end());

    std::lock_guard lock(m_mutex);
    std::vector<SensorReadout> result;

    /* Not enough data*/
    if (m_stream_buffer.size() < MINIMUM_TOTAL_SIZE)
        return result;

    /* Find sync */
    auto start = find_sync(m_stream_buffer);
    if (start < 0)
        return result;

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
        if (header.checksum != compute_checksum(&m_stream_buffer[msg_pos], HEADER_LENGTH))
            throw std::runtime_error("invalid header checksum");

        /* Check we have the whole message */
        if (m_stream_buffer.size() - msg_pos < header.message_size)
            break;

        SensorReadout readout;
        readout.sensor_id = header.sensor_id;
        readout.device_id = header.device_id;
        readout.sequence_no = header.sequence_no;
        for (size_t i = 0; i < header.no_readouts; ++i) {
            auto seconds = to_uint64(&m_stream_buffer[msg_pos + DATA_POS + 24 * i]);
            auto milliseconds = to_uint64(&m_stream_buffer[msg_pos + DATA_POS + 24 * i + 8]);
            readout.time.push_back(seconds + 1E-6 * milliseconds);
            readout.readouts.push_back(to_double(&m_stream_buffer[msg_pos + DATA_POS + 24 * i + 2 * 8]));
        }
        result.push_back(readout);

        if (to_uint32(&m_stream_buffer[msg_pos + DATA_POS + 24 * (int)header.no_readouts]) != compute_checksum(&m_stream_buffer[msg_pos], header.message_size))
            throw std::runtime_error("invalid packet checksum");

        msg_pos += header.message_size;
    }

    /* Remove things that we have used */
    m_stream_buffer.erase(m_stream_buffer.begin(),
                          m_stream_buffer.begin() + msg_pos);

    return result;

}

} // namespace udaq::devices::safibra
