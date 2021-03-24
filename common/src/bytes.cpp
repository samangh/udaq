#include <bytes.h>

uint16_t udaq::common::bytes::to_uint16(const uint8_t *buff, Endianess endian)
{
    if (endian == Endianess::LittleEndian)
        return buff[0] | buff[1] << 8;
    else
        return buff[1] | buff[0] << 8;
}

uint32_t udaq::common::bytes::to_uint32(const uint8_t *buff, udaq::common::bytes::Endianess endian)
{
    if (endian == Endianess::LittleEndian)
        return buff[0] | buff[1] << 8 | buff[2] << 16 | buff[3] << 24;
    else
        return buff[3] | buff[2] << 8 | buff[1] << 16 | buff[0] << 24;

}
