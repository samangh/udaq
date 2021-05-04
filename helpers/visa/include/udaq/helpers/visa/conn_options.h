#pragma once

#include <string>

namespace udaq::visa {

typedef std::string visa_address_t;

struct ConnectionParameters {
    ConnectionParameters()
    {

    }
    ConnectionParameters(const visa_address_t& visa_address):address(visa_address)
    {

    }
    ConnectionParameters(const char* visa_address):address(visa_address){}

    visa_address_t address;
    char termination_character = '\n';
    bool termination_character_needed = true;
};

}
