#include <cstdlib>
#include <vector>
#include <string>
#include <bytes.h>

namespace udaq::devices::safibra {

const size_t HEADER_LENGTH = 80;
const size_t DATA_POS = HEADER_LENGTH;

const size_t SYNC_SIZE = 3;
const size_t DEVICE_ID_POSITION = 4;
const size_t DEVICE_ID_SIZE = 32;
const size_t SENSOR_ID_SIZE = 32;
const size_t PACKET_COUNTER_SIZE = 2;
const size_t PACKET_READOUT_COUNTER_SIZE = 2;
const size_t PACKET_BYTES_SIZE = 4;
const size_t HEADER_CHECKSUM_SIZE = 4;
const size_t PACKET_CHECKSUM_SIZE = 4;

const size_t MINIMUM_TOTAL_SIZE = HEADER_LENGTH + PACKET_CHECKSUM_SIZE + 24;

const size_t SENSOR_ID_POSITION = DEVICE_ID_POSITION + DEVICE_ID_SIZE;
const size_t PACKET_COUNTER_POSITION = SENSOR_ID_POSITION + SENSOR_ID_SIZE;
const size_t PACKET_READOUT_COUNTER_POSITION = PACKET_COUNTER_POSITION+PACKET_COUNTER_SIZE;
const size_t PACKET_BYTES_POSITION = PACKET_READOUT_COUNTER_POSITION + PACKET_READOUT_COUNTER_SIZE;
const size_t HEADER_CHECKSUM_POSITION = PACKET_BYTES_POSITION + PACKET_BYTES_SIZE;


struct Header {
 Header(const std::vector<char>& data) {
     using namespace udaq::common::bytes;

        device_id = data[DEVICE_ID_POSITION];
        sensor_id = data[SENSOR_ID_POSITION];
        sequence_no = to_uint16((unsigned char *)data[PACKET_COUNTER_POSITION]);
        no_readouts =to_uint16((unsigned char *)data[PACKET_READOUT_COUNTER_POSITION]);
        message_size = to_uint32((unsigned char *)data[PACKET_BYTES_POSITION]);
    }

    std::string device_id;
    std::string sensor_id;
    uint16_t sequence_no;
    uint16_t no_readouts;
    uint32_t message_size;
    char checksum[4];
};


}
