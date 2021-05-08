#pragma once

#include <stdexcept>
#include <string>

namespace udaq {

class Exception: public std::runtime_error {
public:
    Exception(const std::string& msg):std::runtime_error(msg){}
};

}
