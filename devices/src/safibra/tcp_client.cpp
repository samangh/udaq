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

void close_handle(uv_handle_s* handle, void* args)
{
 uv_close(handle, nullptr);
}

namespace udaq::devices::safibra {

safibra_tcp_client::safibra_tcp_client(on_error_cb_t on_error_cb):m_on_error_cb(on_error_cb){

}

void safibra_tcp_client::connect(const std::string& address, const int port) {
    int err=0;

    /* Loop */
    m_loop.data = this;

    /* Create destination */
    struct sockaddr_in dest;
    err = uv_ip4_addr(address.c_str(), port, &dest);
    if ( err !=0)
        throw new std::runtime_error(uv_strerror(err));

    /* Create socket */
    auto m_sock = std::make_unique<uv_tcp_t>();
    err = uv_tcp_init(&m_loop, m_sock.get());
    if ( err !=0)
        throw new std::runtime_error(uv_strerror(err));

    /* Create connection*/
    auto m_conn = std::make_unique<uv_connect_t>();
    m_conn->data=this;

    uv_tcp_keepalive(m_sock.get(), 1, 60);
    err = uv_tcp_connect(m_conn.get(), m_sock.get(), (const struct sockaddr*)&dest, on_connect);
    if ( err !=0)
        throw new std::runtime_error(uv_strerror(err));

    m_thread = std::thread([&](){
       while (true)
       {
            uv_run(&m_loop, UV_RUN_DEFAULT);

            /* The following loop closing logic is from guidance from
             * https://stackoverflow.com/questions/25615340/closing-libuv-handles-correctly */

            /* stop the loop */
            uv_stop(&m_loop);

            /*  If there are any loops that are not closing:
             *
             *  - Use uv_walk and call uv_close on the handles;
             *  - Run the loop again with uv_run so all close callbacks are
             *    called and you can free the memory in the callbacks */
            if (uv_loop_close(&m_loop) > 0)
                uv_walk(&m_loop, &close_handle, nullptr);
            else
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
    return m_thread.joinable();
}

safibra_tcp_client::~safibra_tcp_client()
{
    disconnect();
}

void safibra_tcp_client::on_read(uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf)
{
    if(nread >= 0) {
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

}






