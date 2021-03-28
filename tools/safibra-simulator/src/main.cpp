// blocking_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)

// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Modified by S. Ghannadzadeh

#define BOOST_BIND_GLOBAL_PLACEHOLDERS

#include <cstdlib>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <thread>
#include <chrono>

using boost::asio::ip::tcp;

void write_data(boost::asio::ip::tcp::socket& socket)
{
    using namespace std::chrono_literals;

        for (int i=0; i<100; ++i)
        {
            std::string device_id = "simulated_device0";
            auto device_id_padded =  device_id.append(std::string((32 - strlen(device_id.c_str())), '0')).c_str();

            std::string sensor_id = "simulated_sensor0";
            auto sensor_id_padded = device_id.append(
                std::string((32 - strlen(sensor_id.c_str())), '0')).c_str();

            std::vector<char> saf_header;
            /* SYNC */
            saf_header.push_back(0x55);
            saf_header.push_back(0x00);
            saf_header.push_back(0x55);

            /* type */
            saf_header.push_back(0x00);

            /*device and sensor ids*/
            saf_header.insert(saf_header.end(), device_id_padded, device_id_padded + 32);
            saf_header.insert(saf_header.end(), sensor_id_padded, sensor_id_padded + 32);

            saf_header.push_back(1); /* sequence no */
            saf_header.push_back(1); /* number of readouts */

            boost::asio::write(socket, boost::asio::buffer(saf_header));
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

        boost::asio::io_service io_service;
        boost::asio::ip::tcp::socket socket(io_service);

        socket.connect(endpoint);
        write_data(socket);
        socket.close();

    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
