#pragma once

#include <string>
#include <sys/types.h>

namespace udaq
{

typedef std::string uri_t;
typedef uint16_t port_t;

enum class ResultType
{
    Integer,
    Double,
    UnitTime,
    String
};

typedef struct {
    std::string command;
    std::string description;
    std::string unit;
    ResultType result_type;
} MeasurementCapability;

}
