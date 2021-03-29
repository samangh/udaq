#pragma once
#include <vector>
#include <cstdlib>

namespace udaq::devices::safibra {

struct SensorReadout {
    std::vector<double> time;
    std::vector<double> readouts;
};

}
