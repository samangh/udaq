#include <udaq/wrappers/c/safibra_interrogator.h>
#include <stdio.h>

//The following is needed for sleep

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#if WIN32
void sleep(DWORD dwMilliseconds) {
    Sleep(dwMilliseconds);
}

#endif


void on_error(const char* msg) {
    printf("error: %s\n", msg);
};

void analyse(safibra_packet_buffer buffer)
{
    for (int i = 0; i < buffer.length; i++)
    {
        printf("device_id: %s\n", buffer.packets[i].device_id);
        printf("  sensor_id: %s\n", buffer.packets[i].sensor_id);
        for (int k=0; k < buffer.packets[i].length; k++)
        {
            printf("    time: %f\n", buffer.packets[i].time[k]);
            printf("    wavelength: %f\n", buffer.packets[i].readouts[k]);
        }
    }
    safibra_free_buffer(buffer);
};

void empty(void)
{
};

int main(void)
{
    safibra_client client = safibra_create_client(on_error, empty, empty, empty, empty, empty);
    safibra_start(client, 5555);
    sleep(5000);
    analyse(safibra_get_buffer(client));
    safibra_is_running(client);
    safibra_free_client(client);
    return 0;
}
