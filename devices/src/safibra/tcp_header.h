#include <cstdlib>
#include <vector>
#include <string>
#include <bytes.h>
#include <udaq/devices/safibra/fbg_reading.h>

#include "tcp_data_offsets.h"

namespace udaq::devices::safibra {


struct Header {
 Header(const std::vector<unsigned char>& data, size_t start_pos) {
     if (data.size() - start_pos <  HEADER_LENGTH)
         throw new std::invalid_argument(" Buffer too short for a Safibra interrogator stream header");

        device_id = data[start_pos + DEVICE_ID_POSITION];
        sensor_id = data[start_pos + SENSOR_ID_POSITION];
        sequence_no = udaq::common::bytes::to_uint16(&data[start_pos + PACKET_COUNTER_POSITION]);
        no_readouts = udaq::common::bytes::to_uint16(&data[start_pos + PACKET_READOUT_COUNTER_POSITION]);
        message_size = udaq::common::bytes::to_uint32(&data[start_pos + PACKET_BYTES_POSITION]);
    }

    std::string device_id;
    std::string sensor_id;
    uint16_t sequence_no;
    uint16_t no_readouts;
    uint32_t message_size;
    char checksum[4];
};


struct Message {
    Header header;
    FBGReading readouts;
};

}
