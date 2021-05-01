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
const std::string SendLongString ="SEND_LONG_STRING";

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

            sock->read_some(boost::asio::buffer(data), error);
            if (error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer.
            else if (error)
                throw boost::system::system_error(error); // Some other error.

            input = remove_whitespace(data);

            if (input == QueryIDN)
            {
                response=ID;
                goto respond;
            }

            if (input == SendLongString)
            {
                std::string s;
                for (int i=0; i < 2010; i++)
                    s = s + "0";
                response=s;
                goto respond;
            }

            response = "Error: command not found";
        respond:
            std::cout << "Received: " << input << std::endl;
            std::cout << "Sent: " << response << std::endl;
            auto to_send = response + "\n";

            boost::asio::write(*sock, boost::asio::buffer(to_send));
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
