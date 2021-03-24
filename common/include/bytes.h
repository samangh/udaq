#pragma once

#include <cstdint>

namespace udaq::common::bytes {

enum class Endianess
{
    LittleEndian,
    BigEndian
};

static uint16_t to_uint16(const uint8_t *buff, Endianess endian = Endianess::LittleEndian);
static uint32_t to_uint32(const uint8_t *buff, Endianess endian = Endianess::LittleEndian);
static int32_t to_int32(const uint8_t *buff, Endianess endian = Endianess::LittleEndian);
static uint64_t to_uint64(const uint8_t *buff, Endianess endian = Endianess::LittleEndian);
static int64_t to_int64(const uint8_t *buff, Endianess endian = Endianess::LittleEndian);

}
