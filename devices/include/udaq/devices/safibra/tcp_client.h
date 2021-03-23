#pragma once

#include <uv.h>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <functional>

namespace udaq::devices::safibra {

typedef std::function<void(const std::string& msg)> on_error_cb_t;

class safibra_tcp_client{
public:
    safibra_tcp_client(on_error_cb_t on_error_cb);
    void connect(const std::string& address, const int port);
    void disconnect();
    bool is_running();
    ~safibra_tcp_client();
private:
    static void on_read(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf);
    static void on_connect(uv_connect_t* connection, int status);
    static void alloc_cb(uv_handle_t* handle, size_t size, uv_buf_t* buf);
    static void on_close(uv_handle_t* handle);

    void on_error();

    uv_loop_t m_loop;
    std::mutex m_mutex;
    std::thread m_thread;
    on_error_cb_t m_on_error_cb;
    std::atomic<bool> m_end;
    
    std::unique_ptr<uv_tcp_t> m_sock; /* Socket used for connection */
    std::unique_ptr<uv_connect_t> m_conn; /* UV connection object */

};


}
