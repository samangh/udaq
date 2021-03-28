#pragma once

#include <cstdint>
#include <type_traits>
namespace udaq::common::bytes {

enum class Endianess
{
    LittleEndian,
    BigEndian
};

uint16_t to_uint16(const uint8_t *buff, Endianess endian = Endianess::LittleEndian);
uint32_t to_uint32(const uint8_t *buff, Endianess endian = Endianess::LittleEndian);
int32_t to_int32(const uint8_t *buff, Endianess endian = Endianess::LittleEndian);
uint64_t to_uint64(const uint8_t *buff, Endianess endian = Endianess::LittleEndian);
int64_t to_int64(const uint8_t *buff, Endianess endian = Endianess::LittleEndian);
double to_double(const uint8_t *buff, Endianess endian = Endianess::LittleEndian);

template <typename T>
static T swap_endian(const T val, typename std::enable_if<std::is_arithmetic<T>::value, std::nullptr_t>::type = nullptr);


}
