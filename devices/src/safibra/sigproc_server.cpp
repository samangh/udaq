#include <udaq/devices/safibra/sigproc_server.h>

#include "tcp_client.h"

udaq::devices::safibra::SigprogServer::SigprogServer(
    on_error_cb_t on_error_cb, on_client_connected_cb_t on_client_connected_cb,
    on_client_disconnected_cb_t on_client_disconnected_cb,
    on_start_cb_t on_start, on_stop_cb_t on_stop)
    : m_client(std::make_unique<safibra_tcp_client>(on_error_cb,
        on_client_connected_cb,
        on_client_disconnected_cb,
        on_start, 
        on_stop)) {}

udaq::devices::safibra::SigprogServer::~SigprogServer() {}

void udaq::devices::safibra::SigprogServer::start(const int port) {
    m_client->start(port);
}

void udaq::devices::safibra::SigprogServer::stop() { m_client->stop(); }

bool udaq::devices::safibra::SigprogServer::is_running() { return m_client->is_running(); }
