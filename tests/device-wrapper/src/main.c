#include <udaq/devices/wrappers/safibra_interrogator.h>

void on_error(const char* msg, int size) {

};

void on_data(safibra_r* buffer, size_t length)
{

};

void empty()
{

};

int main(void)
{
    void* client = safibra_create_client(&on_error, &empty, &empty, &empty, &on_data);
    safibra_start(client, 5555);
    return 0;
};
