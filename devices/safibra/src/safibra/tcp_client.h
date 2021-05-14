#pragma once

#include <uv.h>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <functional>
#include <memory>


namespace udaq::devices::safibra {


class safibra_tcp_client{
public:
    typedef std::function<void(const std::string &msg)> on_error_cb_t;
    typedef std::function<void(void)> on_client_connected_cb_t;
    typedef std::function<void(void)> on_client_disconnected_cb_t;
    typedef std::function<void(void)> on_start_cb_t;
    typedef std::function<void(void)> on_stop_cb_t;
    typedef std::function<void(const uint8_t* buff, size_t length)> on_data_available_cb_t; /* Callback should NOT free the buffer */

    safibra_tcp_client(on_error_cb_t on_error_cb,
                       on_client_connected_cb_t on_client_connected_cb,
                       on_client_disconnected_cb_t on_client_disconnected_cb,
                       on_start_cb_t on_start,
                       on_stop_cb_t on_stop,
                       on_data_available_cb_t on_data_available_cb);
    void start(const int port);
    void stop();
    bool is_running() const;
    int  number_of_clients() const;
    ~safibra_tcp_client();

private:
    static void on_read(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf);
    static void on_new_connection(uv_stream_t *stream, int status);
    static void alloc_cb(uv_handle_t* handle, size_t size, uv_buf_t* buf);
    static void on_client_disconnected(uv_handle_t* handle);
    void on_error(const std::string &message);

    uv_loop_t m_loop;
    std::thread m_thread;
    mutable std::shared_mutex m_mutex;          /* Mutex for getting data*/

    on_error_cb_t m_on_error_cb; /* Called in case of errors after connect(...) */
    on_client_connected_cb_t m_on_client_connected_cb;
    on_client_disconnected_cb_t m_on_client_disconnected_cb;
    on_start_cb_t m_on_start_cb;
    on_stop_cb_t m_on_stop_cb;
    on_data_available_cb_t m_on_data_available;

    int m_number_of_connected_clients;

    std::unique_ptr<uv_tcp_t> m_sock; /* Socket used for connection */
    std::unique_ptr<uv_connect_t> m_conn; /* UV connection object */
    std::unique_ptr<uv_async_t> m_async; /* For stopping the loop */
};


}
