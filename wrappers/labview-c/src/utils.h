#include <udaq/wrappers/labview-c/globals.h>

#include <string>

MgErr PopulateStringHandle(LStrHandle handle, const std::string& text);
LStrHandle CreateStringHandle(const std::string& text, MgErr& err);

arr1DH create_arr1DH(MgErr& err, size_t array_length);
