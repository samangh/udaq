#include "utils.h"

LStrHandle CreateStringHandle(const std::string& text, MgErr& err)
{
    auto length = text.size()+1;
    auto newStringHandle = (LStrHandle)DSNewHandle(sizeof(int32) + length * sizeof(uChar));

    err = PopulateStringHandle(newStringHandle, text);
    return newStringHandle;
}

arr1DH create_arr1DH(MgErr& err, size_t array_length)
{

    auto handle = (arr1DH)DSNewHandle(sizeof(arr1D));

    err = NumericArrayResize(iL, 1, (UHandle*)&handle, array_length);
    if (err != noErr)    
        return handle;
    
    (*handle)->dimSize = array_length;
    return handle;
}

MgErr PopulateStringHandle(LStrHandle strHandle, const std::string& text)
{

    auto new_length = text.size()+1;
    auto old_length = (*strHandle)->cnt;

    auto err = NumericArrayResize(uB, old_length, (UHandle*)&strHandle, new_length);
    if (err != noErr)
        return err;

    //Empties the buffer
    memset(LStrBuf(*strHandle), '\0', new_length);

    //Fills the string buffer with stringData
    snprintf((char*)LStrBuf(*strHandle), new_length, "%s", text.c_str());

    //Informs the LabVIEW string handle about the size of the size
    LStrLen(*strHandle) = new_length;

    return noErr;
}
