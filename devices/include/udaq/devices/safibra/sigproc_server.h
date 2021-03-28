#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>
#include <cstdint>

#include "sensor_readout.h"

namespace udaq::devices::safibra {

class safibra_tcp_client;

class SigprogServer {
  public:
    typedef std::function<void(const std::string &msg)> on_error_cb_t;
    typedef std::function<void(void)> on_client_connected_cb_t;
    typedef std::function<void(void)> on_client_disconnected_cb_t;
    typedef std::function<void(void)> on_start_cb_t;
    typedef std::function<void(void)> on_stop_cb_t;

    SigprogServer(on_error_cb_t on_error_cb,
                  on_client_connected_cb_t on_client_connected_cb,
                  on_client_disconnected_cb_t on_client_disconnected_cb,
                  on_start_cb_t on_start, on_stop_cb_t on_stop);
    ~SigprogServer();

    void start(const int port);
    void stop();
    bool is_running();
  private:
    void on_data_available_cb(const uint8_t*, size_t length);
    std::vector<unsigned char> m_stream_buffer;
    std::map<std::string, SensorReadout> m_data_buffer;
    std::unique_ptr<safibra_tcp_client> m_client;
};

}
