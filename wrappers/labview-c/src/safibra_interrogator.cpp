#include <udaq/wrappers/labview-c/safibra_interrogator.h>

#include <udaq/devices/safibra/sigproc_server.h>

#include "utils.h"

safibra_client safibra_create_client(InstanceDataPtr* ptr,
                                     LVUserEventRef* erro_cb,
                                     LVUserEventRef* client_connected_cb,
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
    try{
        auto a = (udaq::devices::safibra::SigprogServer*)(client);
        a->start(port);
    } catch(const std::exception& ex)
    {
        PopulateStringHandle(errorMsg, ex.what());
        return ncNetErr;
    }
    return noErr;
}

void safibra_free_buffer(safibra_packet_buffer buffer) {
    for (size_t i = 0; i < buffer.length; i++)
    {
        delete[] buffer.packets[i].device_id;
        delete[] buffer.packets[i].sensor_id;
        delete[] buffer.packets[i].readouts;
        delete[] buffer.packets[i].time;
    }
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

MgErr safibra_get_buffer(safibra_client client, arr1DH array)
{
    //auto a = (udaq::devices::safibra::SigprogServer*)client;
    //auto in = a->get_data_buffer();
    //auto buffer = safibra_packet_buffer();

    //buffer.length = in.size();
    //buffer.packets = new safibra_packet[buffer.length];

    //for (size_t i = 0; i < buffer.length; i++)
    //{
    //    const auto& in_readout = in[i];

    //    buffer.packets[i] = safibra_packet();

    //    size_t device_id_length = in_readout.device_id.size() + 1;
    //    buffer.packets[i].device_id = new char[device_id_length];
    //    std::copy_n(in_readout.device_id.c_str(), device_id_length, buffer.packets[i].device_id);

    //    size_t sensor_id_length = in_readout.sensor_id.size() + 1;
    //    buffer.packets[i].sensor_id = new char[sensor_id_length];
    //    std::copy_n(in_readout.sensor_id.c_str(), sensor_id_length, buffer.packets[i].sensor_id);

    //    size_t length = in_readout.readouts.size();

    //    buffer.packets[i].time = new double[length];
    //    std::copy_n(&(in_readout.time)[0], length, buffer.packets[i].time);

    //    buffer.packets[i].readouts = new double[length];
    //    std::copy_n(&(in_readout.readouts)[0], length, buffer.packets[i].readouts);

    //    buffer.packets[i].sequence_no = in_readout.sequence_no;

    //    buffer.packets[i].length = length;
    //}


     
    auto result = NumericArrayResize(iL, (*array)->dimSize, (UHandle*)&array, 10);
    (*array)->dimSize = 10; 

    (*array)->elt[0] = 1;
    (*array)->elt[1] = 2;
    (*array)->elt[2] = 3;
    (*array)->elt[3] = 4;

    return result;
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
