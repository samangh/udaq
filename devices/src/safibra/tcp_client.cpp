#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <system_error>
#include <uv.h>

#include <bytes.h>

#include "tcp_client.h"

namespace udaq::devices::safibra {

    /* Call back for freeing handles after they are closed */
    void free_handle_on_close(uv_handle_t* handle) {
        if (handle != nullptr)
            delete handle;
    }

    void close_handle(uv_handle_s* handle, void*)
    {
        uv_close(handle, nullptr);
    }


safibra_tcp_client::safibra_tcp_client(
    on_error_cb_t on_error_cb, on_client_connected_cb_t on_client_connected_cb,
    on_client_disconnected_cb_t on_client_disconnected_cb,
    on_start_cb_t on_start, on_stop_cb_t on_stop, on_data_available_cb_t on_data_available_cb)
    : m_on_error_cb(on_error_cb),
      m_on_client_connected_cb(on_client_connected_cb),
      m_on_client_disconnected_cb(on_client_disconnected_cb),
      m_on_start_cb(on_start), m_on_stop_cb(on_stop), m_on_data_available(on_data_available_cb) {
}

void safibra_tcp_client::start(const int port) {
    /* setup UV loop */
    uv_loop_init(&m_loop);
    m_loop.data = this;

    int err=0;

    /* Setup handle for stopping the loop */
    m_async = std::make_unique<uv_async_t>();
    uv_async_init(&m_loop, m_async.get(),
                  [](uv_async_t *handle) { uv_stop(handle->loop); });

    /* Create address */
    struct sockaddr_in dest;
    err = uv_ip4_addr("0.0.0.0", port, &dest);
    if ( err !=0)
        throw new std::runtime_error(uv_strerror(err));

    /* Create socket */
    m_sock = std::make_unique<uv_tcp_t>();
    err = uv_tcp_init(&m_loop, m_sock.get());
    if ( err !=0)
        throw new std::runtime_error(uv_strerror(err));

    /* Bind socket and address */
    err = uv_tcp_bind(m_sock.get(), (const struct sockaddr*)&dest, 0);
    if ( err !=0)
        throw new std::runtime_error(uv_strerror(err));

    /* Start listening */
    err = uv_listen((uv_stream_t *)m_sock.get(), 20, on_new_connection);
    if (err != 0)
        throw new std::runtime_error(uv_strerror(err));

    m_thread = std::thread([&](){
        m_on_start_cb();
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
                break;
        }
        m_on_stop_cb();
    });
}

void safibra_tcp_client::stop()
{
    // Signal the thread that it should stop
    //m_end = true;

    if (uv_loop_alive(&m_loop))
        uv_async_send(m_async.get());

    // join and wait for the thread to end.
    // Note: A thread that has finished executing code, but has not yet been joined
    //        is still considered an active thread of execution and is therefore joinable.
    if (m_thread.joinable())
        m_thread.join();

    m_on_stop_cb();
}

bool safibra_tcp_client::is_running()
{
    return m_thread.joinable()  && (uv_loop_alive(&m_loop) != 0);
}

safibra_tcp_client::~safibra_tcp_client()
{
    if (is_running())
        stop();
}

void safibra_tcp_client::on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    /* Take ownership of the buffer, as it is now ours. */
    auto _buffer = std::unique_ptr<char>(buf->base);

    auto a = (safibra_tcp_client*)client->loop->data;

    /* Check for disconnection*/
    if(nread < 0) {
        uv_close((uv_handle_t *)client, free_handle_on_close);
        if (nread != UV_EOF)
            a->on_error(uv_strerror((int)nread));
        else
            a->m_on_client_disconnected_cb();
        return;
    }

    /* Call callback, callback should not free the buffer */
    a->m_on_data_available((const uint8_t*)buf->base, nread);
}

void safibra_tcp_client::on_new_connection(uv_stream_t *server, int status) {
    auto a = (safibra_tcp_client *)(server->loop->data);

    if (status < 0) {
        a->on_error(uv_strerror((int)status));
        return;
    }

    uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(&(a->m_loop), client);

    if (uv_accept(server, (uv_stream_t *)client) == 0) {
        uv_read_start((uv_stream_t *)client, alloc_cb, on_read);
        a->m_on_client_connected_cb();
    } else {
        /* for some reason we could not accept a connection */
        uv_close((uv_handle_t *)client, free_handle_on_close);
        a->on_error(uv_strerror(status));
        return;
    }
}

void safibra_tcp_client::alloc_cb(uv_handle_t *, size_t size, uv_buf_t *buf) {
    /* Generate buffer for the handle */
    *buf = uv_buf_init((char*)malloc(size), (unsigned int)size);
}

void safibra_tcp_client::on_error(const std::string& message)
{
    m_on_error_cb(message);
}

}
