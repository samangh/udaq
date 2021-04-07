#include <udaq/devices/wrappers/safibra_interrogator.h>
#include <unistd.h>

void on_error(const char* msg, int size) {

};

void on_data(safibra_r* buffer, size_t length)
{
    printf("data\n");
};

void empty()
{
};

int main(void)
{
    void* client = safibra_create_client(&on_error, &empty, &empty, &empty, &empty, &on_data);
    safibra_start(client, 5555);
    for (;;)
        sleep(50);
    return 0;
};
