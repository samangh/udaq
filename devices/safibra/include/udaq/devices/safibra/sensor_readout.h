#pragma once
#include <vector>
#include <cstdlib>
#include <string>

namespace udaq::devices::safibra {

struct SensorReadout {
    std::string sensor_id;
    std::string device_id;
    std::vector<double> time;
    std::vector<double> readouts;
    uint16_t sequence_no;
};

}
