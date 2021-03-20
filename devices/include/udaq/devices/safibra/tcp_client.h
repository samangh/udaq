#pragma once

#include <uv.h>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <functional>

namespace udaq::devices {

typedef std::function<void(const std::string& msg)> on_error_cb_t;

class safibra_tcp_client{
public:
    safibra_tcp_client(std::vector<double>& time, std::vector<double>& wavelengths);
    void connect(on_error_cb_t on_error_cb);
    void disconnect();
    bool is_running();
    ~safibra_tcp_client();
private:
    static void on_read(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf);
    static void on_connect(uv_connect_t* connection, int status);
    static void alloc_cb(uv_handle_t* handle, size_t size, uv_buf_t* buf);
    static void on_close(uv_handle_t* handle);

    void on_error();

    void startConn(const std::string& host, int port);
    uv_loop_t *loop;
    std::mutex m_mutex;
    std::thread m_thread;
    std::vector<double>& m_time;
    std::vector<double>& m_wavelengths;
    on_error_cb_t m_on_error_cb;
};
}
