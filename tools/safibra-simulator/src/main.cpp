#define BOOST_BIND_GLOBAL_PLACEHOLDERS

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <CLI/CLI.hpp>
#include <boost/asio.hpp>

#include <bytes.h>
#include <udaq/common/accurate_sleeper.h>

static boost::asio::io_service io_service;

/* True if async write in progress */
static std::atomic<bool> async_writing_in_progress;

/* Buffer to store data to send */
static std::vector<unsigned char> buffer;

/* Buffer that is passed to ASIO for async write */
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
void add_to_byte_array(std::vector<uint8_t>& array, T to_add)
{
    auto bytes = udaq::common::bytes::to_bytes(to_add, udaq::common::bytes::Endianess::LittleEndian);
    array.insert(array.end(), bytes.begin(), bytes.end());
}

void asio_work_done(const boost::system::error_code& ec, std::size_t)
{
    async_writing_in_progress = false;
    buffer_async.clear();

    if (ec.value() != 0)
        throw std::runtime_error(ec.message());
}

void write_data(boost::asio::ip::tcp::socket& socket, int readings_per_packet, int no_sensors, double sample_rate, udaq::common::AccurateSleeper::Strategy strategy)
{
    /* Set rate */
    int internval_ns =  static_cast<int>(1E9 / sample_rate); //cast to silent compiler warning
    udaq::common::AccurateSleeper sleeper;
    sleeper.set_interval(internval_ns, strategy);

    std::vector<std::string> sensor_names;
    for (int i=0; i < no_sensors; i++)
        sensor_names.push_back("sensor"+std::to_string(i));

    /* i is sequence number */
    for (int i = 0; ; ++i)
    {
        std::vector<uint64_t> seconds;
        std::vector<uint64_t> microseconds;
        std::vector<double> wavelengths;

        for (int k = 0; k < readings_per_packet; ++k)
        {
            sleeper.sleep();
            auto t = std::chrono::high_resolution_clock::now().time_since_epoch();
            auto seconds_ = std::chrono::duration_cast<std::chrono::seconds>(t).count();
            auto microseconds_ = std::chrono::duration_cast<std::chrono::microseconds>(t).count() % 1000000;

            seconds.push_back(seconds_);
            microseconds.push_back(microseconds_);
            wavelengths.push_back(sin(std::chrono::duration_cast<std::chrono::microseconds>(t).count() * 1E-6 * 3.14 /5));
        }

        for (const auto& sensor_name : sensor_names)
        {
            std::string device_id = "simulated_device0";
            auto sensor_id = sensor_name;

            auto device_id_padded = device_id.append(std::string((32 - strlen(device_id.c_str())), '\0')).c_str();
            auto sensor_id_padded = sensor_id.append(std::string((32 - strlen(sensor_id.c_str())), '\0')).c_str();

            /* packet size */
            int packet_size = 80 + readings_per_packet * 24 + 4;

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
            add_to_byte_array(saf_header, (uint16_t)readings_per_packet); /* number of readouts */

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
    CLI::App app{"Safibra FBG interrogator simulator. This program will stream data in the same format as a physical Safibra FBG interrogator."};

    unsigned int readings_per_packet;
    std::string ip;
    unsigned short port;
    unsigned int no_sensors;
    double sample_rate;
    bool accurate_rate=false;

    app.add_option("--ip", ip, "IP address to stream data to")->required();
    app.add_option("--port", port, "port number to connect to")->required();
    app.add_option("--sensors", no_sensors, "number of sensors to simulate")->default_val(6);
    app.add_option("--readings-per-packet", readings_per_packet, "number of redings per packet for a single FBG")->default_val(10);
    app.add_option("--rate", sample_rate, "sample rate in Hz (approximate)")->default_val(125);
    app.add_flag("--exact-rate", accurate_rate, "Ensure the specified sample rate (will use CPU on tight loop)");

    CLI11_PARSE(app, argc, argv);

    auto strategy = accurate_rate? udaq::common::AccurateSleeper::Strategy::Spin : udaq::common::AccurateSleeper::Strategy::Sleep;

    try
    {
        auto host = boost::asio::ip::address::from_string(ip);
        boost::asio::ip::tcp::endpoint endpoint(host, port);
        boost::asio::ip::tcp::socket socket(io_service);

        socket.connect(endpoint);

        write_data(socket, readings_per_packet, no_sensors, sample_rate, strategy);
        socket.close();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return -1;
    }
    return 0;
}
