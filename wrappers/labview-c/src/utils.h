#include <udaq/wrappers/labview-c/globals.h>

#include <string>

MgErr PopulateStringHandle(LStrHandle* handle, const std::string& text);
LStrHandle CreateStringHandle(const std::string& text, MgErr& err);

arr1DH_int32 create_arr1DH_int32(MgErr& err, size_t array_length);

arr1DH_double create_arr1DH_double(MgErr& err, size_t array_length);
arr1DH_double create_arr1DH_double(MgErr& err, std::vector<double> data);
void populate_arr1DH_double(MgErr& err, arr1DH_double handle, std::vector<double> data);
