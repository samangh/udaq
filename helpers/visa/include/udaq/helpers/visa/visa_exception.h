#pragma once

#include <exception>
#include <udaq/exception.h>
#include "return_codes.h"

namespace udaq::visa {

class VisaException: public udaq::Exception {
public:
    VisaException(int visa_err_code, const std::string& msg):udaq::Exception(msg),_error_code(visa_err_code)
    {

    }
    VisaException(int visa_err_code):VisaException(visa_err_code, get_visa_code_description(visa_err_code) ){}
    int error_code() const noexcept {
        return _error_code;
    }
private:
    const int _error_code;
};

}
