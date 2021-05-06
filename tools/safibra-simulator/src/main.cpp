#define BOOST_BIND_GLOBAL_PLACEHOLDERS

#include <cstdlib>
#include <iostream>
#include <string>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <thread>
#include <chrono>

#include <ctime>
#include <bytes.h>

#include <cmath>
#include <atomic>

using boost::asio::ip::tcp;
static std::atomic<bool> async_writing_in_progress;
static boost::asio::io_service io_service;

static std::vector<unsigned char> buffer;
static std::vector<unsigned char> buffer_async;
#ifdef _WIN32
#include <timeapi.h>
#endif

uint32_t compute_checksum(std::vector<unsigned char>& message, int message_size) {
    uint32_t *p = (uint32_t *)(&message[0]);

    //Sum until the last byte of the header
    int count = (message_size)/4 - 1;

    uint32_t sum = 0;
    for (int i=0; i < count; ++i)
        sum += p[i];

    return sum;
}

template <typename T>
void add_to_byte_array(std::vector<uint8_t>& array, T to_add, typename std::enable_if<std::is_integral<T>::value, std::nullptr_t>::type = nullptr)
{
    auto bytes = udaq::common::bytes::to_bytes(to_add, udaq::common::bytes::Endianess::LittleEndian);
    array.insert(array.end(), bytes.begin(), bytes.end());
}

void add_to_byte_array(std::vector<uint8_t>& array, double to_add)
{
    auto bytes = udaq::common::bytes::to_bytes(to_add, udaq::common::bytes::Endianess::LittleEndian);
    array.insert(array.end(), bytes.begin(), bytes.end());
}

void asio_work_done(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
    async_writing_in_progress = false;
    buffer_async.clear();

    if (ec.failed())
        throw std::runtime_error(ec.message());
}

void write_data(boost::asio::ip::tcp::socket& socket)
{
    using namespace std::chrono_literals;

    /* number of readouts per packet*/
    int n = 20;
    bool io_started = false;

    /* i is sequenc number */
    for (int i = 0; ; ++i)
    {
        std::this_thread::sleep_for(8ms);
        for (std::string sensor_id : {"sensor0", "sensor1", "sensor2", "sensor3", "sensor4", "sensor5", "sensor6"})
        {
            std::string device_id = "simulated_device0";
            auto device_id_padded = device_id.append(std::string((32 - strlen(device_id.c_str())), '\0')).c_str();

            //std::string sensor_id = "simulated_sensor0";
            auto sensor_id_padded = sensor_id.append(std::string((32 - strlen(sensor_id.c_str())), '\0')).c_str();

            /* packet size */
            int packet_size = 80 + n * 24 + 4;

            std::vector<unsigned char> saf_header;
            /* SYNC */
            saf_header.push_back(0x55);
            saf_header.push_back(0x00);
            saf_header.push_back(0x55);

            /* type */
            saf_header.push_back(0x00);

            /*device and sensor ids*/
            saf_header.insert(saf_header.end(), device_id_padded, device_id_padded + 32);
            saf_header.insert(saf_header.end(), sensor_id_padded, sensor_id_padded + 32);

            add_to_byte_array(saf_header, (uint16_t)i); /* sequenc no */
            add_to_byte_array(saf_header, (uint16_t)n); /* number of readouts */

            add_to_byte_array(saf_header, (uint32_t)packet_size);
            add_to_byte_array(saf_header, (uint32_t)compute_checksum(saf_header, 80));

            for (int k = 0; k < n; ++k)
            {
                auto t = std::chrono::system_clock::now().time_since_epoch();
                auto seconds = std::chrono::duration_cast<std::chrono::seconds>(t).count();
                auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t).count();
                auto ms_safira = 1E3 * (milliseconds - (seconds * 1E3));

                add_to_byte_array(saf_header, (uint64_t)seconds);
                add_to_byte_array(saf_header, (uint64_t)ms_safira);

                add_to_byte_array(saf_header, sin(milliseconds * 1E-3 * 3.14 / 60) + 0.1 * sin(milliseconds * 1E-3 * 3.14));
            }

            add_to_byte_array(saf_header, (uint32_t)compute_checksum(saf_header, packet_size));
            buffer.insert(buffer.end(), saf_header.begin(), saf_header.end());
        }

        if (!async_writing_in_progress)
        {
            async_writing_in_progress = true;
            buffer_async.insert(buffer_async.end(), buffer.begin(), buffer.end());
            buffer.clear();
            boost::asio::async_write(socket, boost::asio::buffer(buffer_async), &asio_work_done);
            io_service.restart();
            io_service.run_one();            
        }



    }
}

int main(int argc, char* argv[])
{
    #ifdef  _WIN32
        timeBeginPeriod(1);
    #endif

    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: safibra-simulator <host> <port>\n";
            return 1;
        }

        auto host = boost::asio::ip::address::from_string(argv[1]);
        int port = atoi(argv[2]);

        boost::asio::ip::tcp::endpoint endpoint(host, port);

        boost::asio::ip::tcp::socket socket(io_service);

        socket.connect(endpoint);
        write_data(socket);
        socket.close();

    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

#ifdef  _WIN32
    timeEndPeriod(1);
#endif

    return 0;
}
