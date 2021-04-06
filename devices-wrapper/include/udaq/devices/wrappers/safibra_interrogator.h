#pragma once

#include <udaq/devices/wrappers/globals.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct safibra_r safibra_r;
struct safibra_r {
	char* sensor_id;
	char* device_id;
	double* time;
	double* readouts;
	size_t length;
};

typedef void (*safibra_error_cb_t)(const char* msg, int size);
typedef void (*safibra_connected_cb)(void);
typedef void (*safibra_disconnected_cb)(void);
typedef void (*safibra_started_listening_cb)(void);
typedef void (*safibra_stopped_listening_cb)(void);
typedef void (*safibra_data)(safibra_r* buffer, size_t length);

DLL_PUBLIC void* safibra_create_client(safibra_error_cb_t* erro_cb, safibra_connected_cb* client_connected_cb,
	safibra_disconnected_cb* client_disconnected_cb, safibra_started_listening_cb* started_listening_cb, safibra_data* data_available_cb);

DLL_PUBLIC void safibra_start(void* client, const int port);
DLL_PUBLIC void safibra_stop(void* client);
DLL_PUBLIC bool safibra_is_running(void* client);


#ifdef __cplusplus
}
#endif
