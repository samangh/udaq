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

namespace udaq::devices {

safibra_tcp_client::safibra_tcp_client(std::vector<double> &time, std::vector<double> &wavelengths):m_time(time),m_wavelengths(wavelengths){
}

void safibra_tcp_client::connect(on_error_cb_t on_error_cb) {
    m_on_error_cb = on_error_cb;
    startConn("127.0.0.1", 8081);
}

void safibra_tcp_client::disconnect()
{
    uv_stop(loop);
    std::cout << "Diconnect called" << std::endl;
    //m_thread.detach();
    if (m_thread.joinable())          
        try {
            m_thread.join();
        } 
        catch (const std::system_error& e) {
            if (e.code() != std::errc::no_such_process)
                throw e;
        }
}

bool safibra_tcp_client::is_running()
{
    return m_thread.joinable();
}

safibra_tcp_client::~safibra_tcp_client()
{
    if (is_running())
           disconnect();
}

void safibra_tcp_client::on_read(uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf)
{
    printf("on_read. %p\n", tcp);
    if(nread >= 0) {
        //printf("read: %s\n", tcp->data);
        printf("read: %s\n", buf->base);
    }
    else
    {
        uv_close((uv_handle_t*)tcp, on_close);
    }

    free(buf->base);
}

void safibra_tcp_client::on_connect(uv_connect_t *connection, int status)
{
    if (status < 0) {
        std::cerr << "Error: failed to connect to the interrogator" << std::endl;
        auto a = (safibra_tcp_client*)(connection->data);
        a->on_error();
        free(connection);
        return;
    }
    std::cout << "Connected: " << connection << status << std::endl;

    uv_stream_t* stream = connection->handle;
    free(connection);
    uv_read_start(stream, &safibra_tcp_client::alloc_cb, &safibra_tcp_client::on_read);
}

void safibra_tcp_client::alloc_cb(uv_handle_t *handle, size_t size, uv_buf_t *buf) {
    *buf = uv_buf_init((char*)malloc(size), size);
}

void safibra_tcp_client::on_close(uv_handle_t *handle)
{
    printf("closed.\n");
}

void safibra_tcp_client::on_error()
{
    std::cerr << "Error: received error" << std::endl;
    m_on_error_cb("Error: could not connect to instrument");
}

void safibra_tcp_client::startConn(const std::string& host, int port) {
    loop = uv_default_loop();
    loop->data = this;

    uv_tcp_t *pSock = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, pSock);
    uv_tcp_keepalive(pSock, 1, 60);

    struct sockaddr_in dest;
    uv_ip4_addr(host.c_str(), port, &dest);

    uv_connect_t *pConn = (uv_connect_t *)malloc(sizeof(uv_connect_t));
    pConn->data=this;
    uv_tcp_connect(pConn, pSock, (const struct sockaddr*)&dest, on_connect);
    m_thread = std::thread(uv_run, loop, UV_RUN_DEFAULT);
}

}






