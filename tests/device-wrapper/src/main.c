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
    for (size_t i = 0; i < buffer.length; i++)
    {
        int last_index =  buffer.packets[i].length -1;
        printf("average interval in packet: %lf\n",(buffer.packets[i].time[last_index]-buffer.packets[i].time[0])/buffer.packets[i].length);

        for (size_t k=0; k < buffer.packets[i].length; k++)
        {
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
    while (true)
    {
        if (safibra_number_of_clients(client)>0)
            analyse(safibra_get_buffer(client));
    }
    safibra_is_running(client);
    safibra_free_client(client);
    return 0;
}
