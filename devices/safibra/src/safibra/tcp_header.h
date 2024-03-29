#include <cstdlib>
#include <vector>
#include <string>
#include <sg/bytes.h>
#include <udaq/devices/safibra/fbg_reading.h>

#include "tcp_data_offsets.h"
#include <iostream>
namespace udaq::devices::safibra {


struct Header {
 Header(const std::vector<unsigned char>& data, size_t start_pos) {
     if (data.size() - start_pos <  HEADER_LENGTH)
        throw std::invalid_argument(" Buffer too short for a Safibra interrogator stream header");

     std::vector<char> device_id_c;
     std::vector<char> sensor_id_c;

     device_id_c.insert(device_id_c.begin(), &data[start_pos + DEVICE_ID_POSITION], &data[start_pos + DEVICE_ID_POSITION] + 32 );
     sensor_id_c.insert(sensor_id_c.begin(), &data[start_pos + SENSOR_ID_POSITION], &data[start_pos + SENSOR_ID_POSITION] + 32 );

     device_id = std::string(device_id_c.data());
     sensor_id = std::string(sensor_id_c.data());
     sequence_no = sg::bytes::to_uint16(&data[start_pos + PACKET_COUNTER_POSITION]);
     no_readouts = sg::bytes::to_uint16(&data[start_pos + PACKET_READOUT_COUNTER_POSITION]);
     message_size = sg::bytes::to_uint32(&data[start_pos + PACKET_BYTES_POSITION]);
     checksum = sg::bytes::to_uint32(&data[start_pos + HEADER_CHECKSUM_POSITION]);
    }

    std::string device_id;
    std::string sensor_id;
    uint16_t sequence_no;
    uint16_t no_readouts;
    uint32_t message_size;
    uint32_t checksum;
};


struct Message {
    Header header;
    FBGReading readouts;
};

}
