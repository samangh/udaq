#pragma once

#include <functional>
#include <map>
#include <string>

#include "tcp_client.h"

namespace udaq::devices::safibra {

struct FBGReadoutBuffer {
    std::vector<uint64_t> seconds;
    std::vector<uint64_t> milliseconds;
    std::vector<double> readout;
};

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
    
    std::map<std::string, FBGReadoutBuffer> ReadOuts;

  private:
    udaq::devices::safibra::safibra_tcp_client m_client;

};

}