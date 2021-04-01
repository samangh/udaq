#pragma once

#include <vector>
#include <cstdint>
#include <type_traits>
#include <uv.h>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <functional>
#include <memory>

namespace udaq::common {

class file_writer{
public:
    typedef std::function<void(const std::string &msg)> error_cb_t;
    typedef std::function<void(void)> started_cb_t;
    typedef std::function<void(void)> stopped_cb_t;

    file_writer(std::string _path,
                error_cb_t on_error_cb,
                 started_cb_t on_client_connected_cb,
                 stopped_cb_t on_client_disconnected_cb);
    ~file_writer();
    void start();
    void stop();
    bool is_running();
    void write(const std::string& msg);
    const std::string path;
private:
    static void on_read(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf);
    static void on_new_connection(uv_stream_t *stream, int status);

    void on_error(const std::string &message);

    static void on_uv_open(uv_fs_t* req);
    static void on_uv_on_write(uv_fs_t* req);
    static void on_uv_on_file_close(uv_fs_t* req);
    static void on_uv_idler_tick(uv_idle_t* handle);

    std::unique_ptr<std::vector<char>> m_buffer_in;
    std::unique_ptr<std::vector<char>> m_buffer_out;
        
    bool m_stop_requested = false;
    bool m_write_pending = false;;

    uv_loop_t m_loop;
    uv_idle_t m_idler;
    std::thread m_thread;
    std::shared_mutex m_mutex;          /* Mutex for getting data*/

    error_cb_t m_error_cb; /* Called in case of errors after connect(...) */
    started_cb_t m_started_cb;
    stopped_cb_t m_stopped_cb;

    uv_fs_t open_req;
    uv_fs_t write_req;
    uv_fs_t close_req;
    uv_buf_t m_uv_buf;

};


}
