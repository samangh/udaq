#pragma once

#include <udaq/wrappers/labview-c/globals.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct safibra_packet_buffer safibra_packet_buffer;
typedef struct safibra_packet safibra_packet;

struct safibra_packet {
	LStrHandle sensor_id;
	LStrHandle device_id;
	double* time;
	double* readouts;
	uint16_t sequence_no;
	size_t length;
};

struct safibra_packet_buffer {
	safibra_packet* packets;
	size_t length;
};

typedef void* safibra_client;
typedef void (*safibra_error_cb_t)(const char* msg);
typedef void (PostLV )(void);
typedef void (*safibra_disconnected_cb)(void);
typedef void (*safibra_started_listening_cb)(void);
typedef void (*safibra_stopped_listening_cb)(void);
/* Call back for when data becomes avilable. The caller is responsible for freeing the buffer by calling safibra_free_buffer() afterwards. */
typedef void (*safibra_data)(void);

LABVIEW_C_WRAPPER_EXPORT MgErr safibra_reserve_memory(InstanceDataPtr* ptr);
LABVIEW_C_WRAPPER_EXPORT MgErr safibra_free_memory(InstanceDataPtr *ptr);


/* Creates a new client for Safibra FBG interrogators. The resultign client must be freed by calling safibra_free_client() afterards. */
LABVIEW_C_WRAPPER_EXPORT safibra_client safibra_create_client(InstanceDataPtr *ptr,
	LVUserEventRef*	 erro_cb,
	LVUserEventRef* client_connected_cb,
	LVUserEventRef* client_disconnected_cb,
	LVUserEventRef* started_listening_cb,
	LVUserEventRef* stopped_listening_cb,
	LVUserEventRef*	 data_available_cb);

LABVIEW_C_WRAPPER_EXPORT MgErr safibra_start(safibra_client client, int port, LStrHandle errorMsg);
LABVIEW_C_WRAPPER_EXPORT void safibra_stop(safibra_client client);
LABVIEW_C_WRAPPER_EXPORT bool safibra_is_running(safibra_client client);

LABVIEW_C_WRAPPER_EXPORT MgErr safibra_get_buffer(safibra_client client, arr1DH handle);

LABVIEW_C_WRAPPER_EXPORT void safibra_free_buffer(safibra_packet_buffer buffer);

LABVIEW_C_WRAPPER_EXPORT int safibra_number_of_clients(safibra_client client);

#ifdef __cplusplus
}
#endif
