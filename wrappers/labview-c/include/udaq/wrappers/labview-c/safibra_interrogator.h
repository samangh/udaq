#pragma once

#include <udaq/wrappers/labview-c/globals.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef X86
    #pragma pack(1)
#endif
typedef struct safibra_packet_buffer safibra_packet_buffer;
typedef struct safibra_packet safibra_packet;

struct safibra_packet {
	char* sensor_id;
	char* device_id;
	double* time;
	double* readouts;
	uint16_t sequence_no;
	size_t length;
};

struct safibra_packet_buffer {
	safibra_packet* packets;
	size_t length;
};
#ifdef X86
    #pragma pack()
#endif

typedef void* safibra_client;
typedef void (*safibra_error_cb_t)(const char* msg);
typedef void (*safibra_connected_cb)(void);
typedef void (*safibra_disconnected_cb)(void);
typedef void (*safibra_started_listening_cb)(void);
typedef void (*safibra_stopped_listening_cb)(void);
/* Call back for when data becomes avilable. The caller is responsible for freeing the buffer by calling safibra_free_buffer() afterwards. */
typedef void (*safibra_data)(void);

/* Creates a new client for Safibra FBG interrogators. The resultign client must be freed by calling safibra_free_client() afterards. */
LABVIEW_C_WRAPPER_EXPORT safibra_client safibra_create_client(safibra_error_cb_t erro_cb,
                                       safibra_connected_cb client_connected_cb,
                                       safibra_disconnected_cb client_disconnected_cb,
                                       safibra_started_listening_cb started_listening_cb,
                                       safibra_stopped_listening_cb stopped_listening_cb,
                                       safibra_data data_available_cb);

LABVIEW_C_WRAPPER_EXPORT void safibra_start(safibra_client client, int port);
LABVIEW_C_WRAPPER_EXPORT void safibra_stop(safibra_client client);
LABVIEW_C_WRAPPER_EXPORT bool safibra_is_running(safibra_client client);

LABVIEW_C_WRAPPER_EXPORT safibra_packet_buffer safibra_get_buffer(safibra_client client);

LABVIEW_C_WRAPPER_EXPORT void safibra_free_buffer(safibra_packet_buffer buffer);
LABVIEW_C_WRAPPER_EXPORT void safibra_free_client(safibra_client client);

LABVIEW_C_WRAPPER_EXPORT int safibra_number_of_clients(safibra_client client);

#ifdef __cplusplus
}
#endif
