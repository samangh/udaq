#include <udaq/wrappers/labview-c/safibra_interrogator.h>

#include <udaq/devices/safibra/sigproc_server.h>

#include "utils.h"

safibra_client safibra_create_client(InstanceDataPtr* ptr,
                                     LVUserEventRef* erro_cb,
                                     LVUserEventRef *client_connected_cb,
                                     LVUserEventRef* client_disconnected_cb,
                                     LVUserEventRef* started_listening_cb,
                                     LVUserEventRef* stopped_listening_cb,
                                     LVUserEventRef* data_available_cb) {
    new(*ptr) udaq::devices::safibra::SigprogServer(
    [erro_cb](const std::string& msg) {
        MgErr er;
        auto s = CreateStringHandle(msg, er);
        if (erro_cb!=nullptr)
            PostLVUserEvent(*erro_cb, &s);
    },
    [client_connected_cb]() {
        int i = 0;
        if (client_connected_cb!=nullptr)
            PostLVUserEvent(*client_connected_cb, &i);
    },
    [client_disconnected_cb]() {
        int i = 0;
        if (client_disconnected_cb!=nullptr)
            PostLVUserEvent(*client_disconnected_cb, &i);
    },
    [started_listening_cb]() {
        int i = 0;
        if (started_listening_cb!=nullptr)
            PostLVUserEvent(*started_listening_cb, &i);
    },
    [stopped_listening_cb]() {
        int i = 0;
        if (stopped_listening_cb!=nullptr)
            PostLVUserEvent(*stopped_listening_cb, &i);
    },
    [data_available_cb]() {
        int i = 0;
        if (data_available_cb!=nullptr)
            PostLVUserEvent(*data_available_cb, &i);
    });
    return *ptr;
}

MgErr safibra_start(safibra_client client, int port, LStrHandle* errorMsg) {
    if (client==NULL)
    {
        PopulateStringHandle(errorMsg, "Null safibra client passed");
        return mgArgErr;
    }

    try{
        auto a = (udaq::devices::safibra::SigprogServer*)(client);
        a->start(port);
    } catch(...)
    {
        ///PopulateStringHandle(errorMsg, ex.what());
        return ncNetErr;
    }
    return noErr;
}

int safibra_number_of_clients(safibra_client client)
{
    auto a = (udaq::devices::safibra::SigprogServer*)(client);
    return a->number_of_clients();
}

bool safibra_is_running(safibra_client client) {
    auto a = (udaq::devices::safibra::SigprogServer*)(client);
    return a->is_running();
}

MgErr safibra_get_buffer(safibra_client client, safibra_bufferH* buffer)
{
    auto a = (udaq::devices::safibra::SigprogServer*)client;
    auto in = a->get_data_buffer();
    auto no_packets =in.size();
    auto buffer_size=sizeof(safibra_bufferH) + (no_packets-1)*sizeof(safibra_packet);

    /* Check if input buffer is empty, if so return nulll */
    if (no_packets==0)
    {
        *buffer= NULL;
        return noErr;
    }

    /* Dispose of output buffer data if there is any */
    if (*buffer!=NULL)
    {
        for(int i=0; i < (**buffer)->dimSize; i++)
        {
            DSDisposeHandle((**buffer)->elt[i].device_id);
            DSDisposeHandle((**buffer)->elt[i].sensor_id);
            DSDisposeHandle((**buffer)->elt[i].readouts);
            DSDisposeHandle((**buffer)->elt[i].time);
        }
        DSDisposeHandle(*buffer);
    }

    *buffer=(safibra_buffer**)DSNewHClr(buffer_size);
    (**buffer)->dimSize=no_packets;

    MgErr err = noErr;

    /*Now create remaining ones*/
    for(size_t i=0; i <no_packets; i++)
    {
        safibra_packet p;
        p.device_id = CreateStringHandle(in[i].device_id, err);
        if (err !=0)
            return err;

        p.sensor_id = CreateStringHandle(in[i].sensor_id, err);
        if (err !=0)
            return err;

        p.time = create_arr1DH_double(err, in[i].time);
        p.readouts = create_arr1DH_double(err, in[i].readouts);
        p.length=in[i].readouts.size();
        p.sequence_no=in[i].sequence_no;

        (**buffer)->elt[i]=p;
    }

    return err;
}

void safibra_stop(safibra_client client) {
    auto a = (udaq::devices::safibra::SigprogServer*)(client);
    a->stop();
}

MgErr safibra_reserve_memory(InstanceDataPtr *ptr)
{
    *ptr =  (void*)DSNewPtr(sizeof(udaq::devices::safibra::SigprogServer));
    return noErr;
}

MgErr safibra_free_memory(InstanceDataPtr *ptr)
{
    if (*ptr==nullptr)
        return noErr;

    if (safibra_is_running(*ptr))
        safibra_stop(*ptr);
    return DSDisposePtr(*ptr);
    *ptr=nullptr;
}

MgErr test(safibra_bufferH* b){

    /* The buffer always comes with at least one element */
    DSSetHSzClr(*b, sizeof(safibra_bufferH) + 9*sizeof(safibra_packet));
    (**b)->dimSize=10;

    MgErr err;

    for(int i=1; i <10; i++)
    {
        safibra_packet p;
        p.device_id = CreateStringHandle("device", err);
        if (err !=0)
            return err;

        p.sensor_id = CreateStringHandle("sensor", err);
        if (err !=0)
            return err;

        p.time = create_arr1DH_double(err, std::vector<double>{0,1,2});
        p.readouts = create_arr1DH_double(err, std::vector<double>{0,1,2});
        p.length=3;
        p.sequence_no=0;

        (**b)->elt[i]=p;
    }

    return noErr;
}
