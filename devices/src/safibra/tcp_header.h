#include <cstdlib>
#include <vector>
#include <string>
#include <bytes.h>
#include "tcp_data_offsets.h"

namespace udaq::devices::safibra {

 /* Returns the position of the header sync if found, -1 otherwise*/
 int find_sync(const std::vector<char> &data) {
    for (int i = 0; i < data.size() - 2; i++)
        if (data[i] == 0x55 && data[i + 1] == 0x00 && data[i] == 0x55)
            return i;

    return -1;
}

struct Header {
 Header(const std::vector<char>& data) {

        device_id = data[DEVICE_ID_POSITION];
        sensor_id = data[SENSOR_ID_POSITION];
        sequence_no = udaq::common::bytes::to_uint16(
            (unsigned char *)data[PACKET_COUNTER_POSITION]);
        no_readouts = udaq::common::bytes::to_uint16(
            (unsigned char *)data[PACKET_READOUT_COUNTER_POSITION]);
        message_size = udaq::common::bytes::to_uint32(
            (unsigned char *)data[PACKET_BYTES_POSITION]);
    }

    std::string device_id;
    std::string sensor_id;
    uint16_t sequence_no;
    uint16_t no_readouts;
    uint32_t message_size;
    char checksum[4];
};

struct Readout {
    std::vector<uint64_t> seconds;
    std::vector<uint64_t> milliseconds;
    double result;
};

struct Message {
    Header header;
    Readout readouts[];
};

}
