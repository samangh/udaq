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

using boost::asio::ip::tcp;

typedef boost::shared_ptr<tcp::socket> socket_ptr;

const int INPUT_BUFFER_LENGTH = 2048;

const std::string ID = "SCPI simulator";
const std::string QueryIDN ="*IDN?";

std::string remove_whitespace(std::string input)
{
            input.erase(std::remove(input.begin(), input.end(), '\n'), input.end());
            input.erase(std::remove(input.begin(), input.end(), '\r'), input.end());
            return input;
}

void session(socket_ptr sock)
{
    std::string response;
    std::string input;

    char data[INPUT_BUFFER_LENGTH];
    boost::system::error_code error;

    try
    {
        for (;;)
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

            saf_header.push_back(1); /* sequench no */
            saf_header.push_back(1); /* number of readouts */

            boost::asio::write(*sock, boost::asio::buffer(saf_header));
            return;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception in thread: " << e.what() << "\n";
    }
}

void server(boost::asio::io_service& io_service, short port)
{
    tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));
    for (;;)
    {
        socket_ptr sock(new tcp::socket(io_service));
        a.accept(*sock);
        boost::thread t(boost::bind(session, sock));
    }
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: scpi-simulator <port>\n";
            return 1;
        }

        boost::asio::io_service io_service;

        using namespace std; // For atoi.
        server(io_service, atoi(argv[1]));
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
