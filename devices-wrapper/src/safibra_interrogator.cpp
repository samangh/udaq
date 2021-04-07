#include <udaq/devices/wrappers/safibra_interrogator.h>

#include <udaq/devices/safibra/sigproc_server.h>


void* safibra_create_client(safibra_error_cb_t* erro_cb,
                            safibra_connected_cb* client_connected_cb,
                            safibra_disconnected_cb* client_disconnected_cb,
                            safibra_started_listening_cb* started_listening_cb,
                            safibra_stopped_listening_cb* stopped_listening_cb,
                            safibra_data* data_available_cb) {

	return new udaq::devices::safibra::SigprogServer([erro_cb](const std::string& msg) {(*erro_cb)(msg.c_str(), msg.length());},
        *client_connected_cb,
        *client_disconnected_cb,
        *started_listening_cb,
        *stopped_listening_cb,
        [data_available_cb](std::vector<udaq::devices::safibra::SensorReadout> in) {
			auto data = new safibra_r[in.size()];

			for (const auto& in_readout : in)
			{
                data[0] = safibra_r();
				
                data[0].device_id = new char[in_readout.device_id.size() + 1];
                std::copy_n(in_readout.device_id.c_str(), in_readout.device_id.size(), data[0].device_id);

                data[0].sensor_id = new char[in_readout.sensor_id.size() + 1];
                std::copy_n(in_readout.sensor_id.c_str(), in_readout.sensor_id.size(), data[0].sensor_id);

                int length = in_readout.readouts.size();

                data[0].time = new double[length];
                std::copy_n(&(in_readout.time)[0], length, data[0].time);

                data[0].readouts = new double[length];
                std::copy_n(&(in_readout.readouts)[0], length, data[0].readouts);

                data[0].length = length;
			}

            (data_available_cb)(data, in.size());
		}
	);
}


void safibra_start(void* client, const int port) {
	auto a = (udaq::devices::safibra::SigprogServer*)client;
	a->start(port);
}
