#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <system_error>
#include <uv.h>

#include <udaq/devices/safibra/tcp_client.h>

const size_t HEADER_LENGTH = 80;
const size_t DATA_POS = HEADER_LENGTH;

const size_t SYNC_SIZE = 3;
const size_t DEVICE_ID_POSITION = 4;
const size_t DEVICE_ID_SIZE = 32;
const size_t SENSOR_ID_SIZE = 32;
const size_t PACKET_COUNTER_SIZE = 2;
const size_t PACKET_READOUT_COUNTER_SIZE = 2;
const size_t PACKET_BYTES_SIZE = 4;
const size_t HEADER_CHECKSUM_SIZE = 4;
const size_t PACKET_CHECKSUM_SIZE = 4;

const size_t MINIMUM_TOTAL_SIZE = HEADER_LENGTH + PACKET_CHECKSUM_SIZE + 24;

const size_t SENSOR_ID_POSITION = DEVICE_ID_POSITION + DEVICE_ID_SIZE;
const size_t PACKET_COUNTER_POSITION = SENSOR_ID_POSITION + SENSOR_ID_SIZE;
const size_t PACKET_READOUT_COUNTER_POSITION = PACKET_COUNTER_POSITION+PACKET_COUNTER_SIZE;
const size_t PACKET_BYTES_POSITION = PACKET_READOUT_COUNTER_POSITION + PACKET_READOUT_COUNTER_SIZE;
const size_t HEADER_CHECKSUM_POSITION = PACKET_BYTES_POSITION + PACKET_BYTES_SIZE;


uint16_t bytes_to_uint32(unsigned char *buffer) {
    return (uint32_t)buffer[0] << 24 | (uint32_t)buffer[1] << 16 |
           (uint32_t)buffer[2] << 8 | (uint32_t)buffer[3];
}

uint16_t bytes_to_uint16(unsigned char *buf) { return (buf[1] << 8) + buf[0]; }

uint64_t bytes_to_uint64_t(unsigned char* buf) {
    unsigned long long ull;
    memcpy(&ull, buf, sizeof(ull));
    return ull;
}


void close_handle(uv_handle_s* handle, void* args)
{
 uv_close(handle, nullptr);
}

struct Header {  
    Header(const std::vector<char>& data) {
        device_id = data[DEVICE_ID_POSITION];
        sensor_id = data[SENSOR_ID_POSITION];
        sequence_no =
            bytes_to_uint16((unsigned char *)data[PACKET_COUNTER_POSITION]);
        no_readouts = bytes_to_uint16(
            (unsigned char *)data[PACKET_READOUT_COUNTER_POSITION]);
        message_size =
            bytes_to_uint32((unsigned char *)data[PACKET_BYTES_POSITION]);
    }

    std::string device_id;
    std::string sensor_id;
    uint16_t sequence_no;
    uint16_t no_readouts;
    uint32_t message_size;
    char checksum[4];
};

struct Result {
    std::vector<uint64_t> seconds;
    std::vector<uint64_t> milliseconds;
    double result;
};

/* Returns the position of the header sync if found, -1 otherwise*/
int find_sync(const std::vector<char> &data) {
    for (int i = 0; i < data.size() - 2; i++)
        if (data[i] == 0x55 && data[i + 1] == 0x00 && data[i] == 0x55)
            return i;

    return -1;
}

double bytes_to_double(const unsigned char* buf) {
    double d;
    memcpy(&d, buf, sizeof d);
    return d;
}

bool get_data(std::vector<char> &data) {
    /* Not enough data*/
    if (data.size() < MINIMUM_TOTAL_SIZE)
        return false;

    /* Find sync */
    auto start = find_sync(data);    
    if (start < 0)
        return false;

    if (start > 0)
        data.erase(data.begin(), data.begin() + start);

    if (data.size() >= HEADER_LENGTH) {
        Header h(data);

        if (data.size() >= h.message_size) {
            Result res;
            for (size_t i=0; i++; i < h.no_readouts) {
                res.seconds.push_back(bytes_to_uint64_t((unsigned char *)data[DATA_POS + 24 * i]));
                res.milliseconds.push_back(bytes_to_uint64_t((unsigned char *)data[DATA_POS + 24 * i + 8]));
                res.result = bytes_to_double((unsigned char *)data[DATA_POS + 24 * i + 8]);
            }
        }
    }

}


namespace udaq::devices::safibra {

safibra_tcp_client::safibra_tcp_client(on_error_cb_t on_error_cb)
    : m_on_error_cb(on_error_cb) {
 }

void safibra_tcp_client::connect(const std::string& address, const int port) {
    /* setup UV loop */
    uv_loop_init(&m_loop);
    m_loop.data = this;
    m_end = false;

    m_readstate = ReadState::WaitingForHeader;

    int err=0;

    /* Create destination */
    struct sockaddr_in dest;
    err = uv_ip4_addr(address.c_str(), port, &dest);
    if ( err !=0)
        throw new std::runtime_error(uv_strerror(err));

    /* Create socket */
    m_sock = std::make_unique<uv_tcp_t>();
    err = uv_tcp_init(&m_loop, m_sock.get());
    if ( err !=0)
        throw new std::runtime_error(uv_strerror(err));

    /* Create connection*/
    m_conn = std::make_unique<uv_connect_t>();
    m_conn->data=this;

    uv_tcp_keepalive(m_sock.get(), 1, 60);
    err = uv_tcp_connect(m_conn.get(), m_sock.get(), (const struct sockaddr*)&dest, on_connect);
    if ( err !=0)
        throw new std::runtime_error(uv_strerror(err));

    m_thread = std::thread([&](){
        while (true) {
            uv_run(&m_loop, UV_RUN_DEFAULT);

            /* The following loop closing logic is from guidance from
             * https://stackoverflow.com/questions/25615340/closing-libuv-handles-correctly
             *
             *  If there are any loops that are not closing:
             *
             *  - Use uv_walk and call uv_close on the handles;
             *  - Run the loop again with uv_run so all close callbacks are
             *    called and you can free the memory in the callbacks */

            /* Close callbacks */
            uv_walk(&m_loop, &close_handle, nullptr);

            /* Check if there are any remaining call backs*/
            if (uv_loop_close(&m_loop) != UV_EBUSY)
                return;
        }
    });
}

void safibra_tcp_client::disconnect()
{
    // Signal the thread that it should stop
    m_end = true;

    // join and wait for the thread to end.
    // Note: A thread that has finished executing code, but has not yet been joined
    //        is still considered an active thread of execution and is therefore joinable.
    if (m_thread.joinable())
        m_thread.join();
}

bool safibra_tcp_client::is_running()
{
    return m_thread.joinable()  && (uv_loop_alive(&m_loop) != 0);
}

safibra_tcp_client::~safibra_tcp_client()
{
    disconnect();
}

void safibra_tcp_client::on_read(uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf)
{   
    /* Take ownership of the buffer, as it is now ours */
    auto _buffer = std::unique_ptr<char>(buf->base);
    auto a = (safibra_tcp_client*)tcp->loop->data;
    auto data = &a->m_data;

    /* If requested to end, then do so */
    if (a->m_end) {
        uv_close((uv_handle_t *)tcp, nullptr);
        return;
    }

    if(nread < 0) {
        uv_close((uv_handle_t *)tcp, nullptr);
        a->on_error("server dropped connection");
        return;
    }

    /* Copy data over to our application buffer*/
    data->insert(data->end(), buf->base, buf->base + nread);

    get_data(*data);
}


void safibra_tcp_client::on_connect(uv_connect_t *connection, int status)
{
    if (status < 0) {
        auto a = (safibra_tcp_client*)(connection->data);
        a->on_error(uv_strerror(status));
        return;
    }

    uv_stream_t* stream = connection->handle;
    uv_read_start(stream, &safibra_tcp_client::alloc_cb, &safibra_tcp_client::on_read);
}

void safibra_tcp_client::alloc_cb(uv_handle_t *handle, size_t size, uv_buf_t *buf) {
    /* Generate buffer for the handle */
    *buf = uv_buf_init((char*)malloc(size), size);
}

void safibra_tcp_client::on_error(const std::string& message)
{
    m_on_error_cb(message);
}

}
