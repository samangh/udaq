#pragma once

#include <vector>
#include <cstdint>
#include <type_traits>

namespace udaq::common::vector {

template<typename T>
void append(std::vector<T>& base, const std::vector<T> to_add)
{
    base.insert(std::end(base), std::begin(to_add), std::end(to_add));
}

template<typename T>
int upper_bound_index(const std::vector<T>& data, T  value)
{
    auto upper = std::upper_bound(data.begin(), data.end(), value);
    return  std::distance(data.begin(), upper);
}

template<typename T>
int lower_bound_index(const std::vector<T>& data, double  value)
{
    auto upper = std::lower_bound(data.begin(), data.end(), value);
    return std::distance(data.begin(), upper);
}

}
