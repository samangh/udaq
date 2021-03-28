#include <cstdlib>
#include <vector>
#include <string>

namespace udaq::devices::safibra {

struct FBGReading
{
    std::vector<uint64_t> seconds;
    std::vector<uint64_t> milliseconds;
    std::vector<double> readings;
};

}
