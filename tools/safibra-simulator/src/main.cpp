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
#include <udaq/common/accurate_sleeper.h>
#include <vector>

using boost::asio::ip::tcp;
static std::atomic<bool> async_writing_in_progress;
static boost::asio::io_service io_service;

static std::vector<unsigned char> buffer;
static std::vector<unsigned char> buffer_async;

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

    /* Set rate to 125 Hz */
    udaq::common::AccurateSleeper sleeper;
    sleeper.set_interval(8ms);

    /* i is sequence number */
    for (int i = 0; ; ++i)
    {
        std::vector<uint64_t> seconds;
        std::vector<uint64_t> microseconds;
        std::vector<double> wavelengths;

        for (int k = 0; k < n; ++k)
        {
            sleeper.sleep();
            auto t = std::chrono::high_resolution_clock::now().time_since_epoch();
            auto seconds_ = std::chrono::duration_cast<std::chrono::seconds>(t).count();
            auto microseconds_ = std::chrono::duration_cast<std::chrono::microseconds>(t).count() % 1000000;

            seconds.push_back(seconds_);
            microseconds.push_back(microseconds_);
            wavelengths.push_back(sin(std::chrono::duration_cast<std::chrono::microseconds>(t).count() * 1E-6 * 3.14 /5));
        }

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

            for (size_t k = 0; k < seconds.size(); k++)
            {
                add_to_byte_array(saf_header, (uint64_t)seconds[k]);
                add_to_byte_array(saf_header, (uint64_t)microseconds[k]);
                add_to_byte_array(saf_header, wavelengths[k]);
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
    return 0;
}
