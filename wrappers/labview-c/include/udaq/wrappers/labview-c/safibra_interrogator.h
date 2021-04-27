#pragma once

#include <udaq/wrappers/labview-c/globals.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "lv_prolog.h"

typedef struct {
    LStrHandle device_id;
	LStrHandle sensor_id;
    arr1DH_double time;
    arr1DH_double readouts;
	uint16_t sequence_no;
    uint32_t length;
} safibra_packet;

typedef struct  {
    int32_t dimSize;
    safibra_packet elt[1];
} safibra_buffer;
typedef safibra_buffer** safibra_bufferH;

#include "lv_epilog.h"


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
LABVIEW_C_WRAPPER_EXPORT MgErr test(safibra_bufferH* b);


/* Creates a new client for Safibra FBG interrogators. The resultign client must be freed by calling safibra_free_client() afterards. */
LABVIEW_C_WRAPPER_EXPORT safibra_client safibra_create_client(InstanceDataPtr *ptr,
    LVUserEventRef*	 erro_cb,
    LVUserEventRef* client_connected_cb,
    LVUserEventRef* client_disconnected_cb,
    LVUserEventRef* started_listening_cb,
    LVUserEventRef* stopped_listening_cb,
    LVUserEventRef*	 data_available_cb);

LABVIEW_C_WRAPPER_EXPORT MgErr safibra_start(safibra_client client, int port, LStrHandle* errorMsg);
LABVIEW_C_WRAPPER_EXPORT void safibra_stop(safibra_client client);
LABVIEW_C_WRAPPER_EXPORT bool safibra_is_running(safibra_client client);

LABVIEW_C_WRAPPER_EXPORT MgErr safibra_get_buffer(safibra_client client, safibra_bufferH* buffer);

LABVIEW_C_WRAPPER_EXPORT int safibra_number_of_clients(safibra_client client);

#ifdef __cplusplus
}
#endif
