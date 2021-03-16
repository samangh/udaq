#include <udaq/types.h>

namespace udaq::devices
{
    class random_wamp
    {
    public:
        random_wamp(std::string router_addr, port_t port);
        void connect_client();
        void disconnect_client();
    private:
    };
}
