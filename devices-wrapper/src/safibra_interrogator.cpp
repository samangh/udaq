#include <udaq/devices/wrappers/safibra_interrogator.h>

#include <udaq/devices/safibra/sigproc_server.h>


safibra_client safibra_create_client(safibra_error_cb_t erro_cb,
                            safibra_connected_cb client_connected_cb,
                            safibra_disconnected_cb client_disconnected_cb,
                            safibra_started_listening_cb started_listening_cb,
                            safibra_stopped_listening_cb stopped_listening_cb,
                            safibra_data data_available_cb) {
    safibra_client a;
    a.client = 	new udaq::devices::safibra::SigprogServer([erro_cb](const std::string& msg) {erro_cb(msg.c_str());},
        client_connected_cb,
        client_disconnected_cb,
        started_listening_cb,
        stopped_listening_cb,
        [data_available_cb](std::vector<udaq::devices::safibra::SensorReadout> in) {
            auto buffer = new safibra_packet_buffer();
            buffer->length = in.size();
            buffer->packets = new safibra_packet[buffer->length];

            for (size_t i=0; i < buffer->length; i++)
			{
                const auto& in_readout = in[i];
                
                buffer->packets[i] = safibra_packet();			               

                size_t device_id_length = in_readout.device_id.size() + 1;
                buffer->packets[i].device_id = new char[device_id_length];
                std::copy_n(in_readout.device_id.c_str(), device_id_length, buffer->packets[i].device_id);

                size_t sensor_id_length = in_readout.sensor_id.size() + 1;
                buffer->packets[i].sensor_id = new char[sensor_id_length];
                std::copy_n(in_readout.sensor_id.c_str(), sensor_id_length, buffer->packets[i].sensor_id);

                size_t length = in_readout.readouts.size();

                buffer->packets[i].time = new double[length];
                std::copy_n(&(in_readout.time)[0], length, buffer->packets[i].time);

                buffer->packets[i].readouts = new double[length];
                std::copy_n(&(in_readout.readouts)[0], length, buffer->packets[i].readouts);

                buffer->packets[i].length = length;
			}

            //data_available_cb(buffer);
		}
	);

    return a;
}


void safibra_start(safibra_client client, const int port) {
	auto a = (udaq::devices::safibra::SigprogServer*)client.client;
	a->start(port);
}

void safibra_free(safibra_packet_buffer* buffer) {
    for (int i = 0; i < buffer->length; i++)
    {
        delete[] buffer->packets[i].device_id;
        delete[] buffer->packets[i].sensor_id;
        delete[] buffer->packets[i].readouts;
        delete[] buffer->packets[i].time;
    }

    delete[] buffer;
}

void safibra_free_client(safibra_client client){
    delete (udaq::devices::safibra::SigprogServer*)client.client;    
}
