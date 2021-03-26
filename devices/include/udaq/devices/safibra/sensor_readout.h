#pragma once
#include <vector>
#include <cstdlib>

namespace udaq::devices::safibra {

struct SensorReadout {
    std::vector<uint64_t> seconds;
    std::vector<uint64_t> milliseconds;
    std::vector<double> readout;
};

}
