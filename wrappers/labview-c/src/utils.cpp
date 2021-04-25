#include "utils.h"
#include <algorithm>

LStrHandle CreateStringHandle(const std::string& text, MgErr& err)
{
    /* Create minimal LStrHandle */
    auto newStringHandle = (LStrHandle)DSNewHandle(sizeof(LStr));
    LStrLen(*newStringHandle) = 1;

    err = PopulateStringHandle(&newStringHandle, text);
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

MgErr PopulateStringHandle(LStrHandle* strHandle, const std::string& text)
{

    auto new_length = text.size()+1;
    auto old_length = (*strHandle != nullptr) ? (**strHandle)->cnt : 1;

    auto err = NumericArrayResize(uB, old_length, (UHandle*)strHandle, new_length);
    if (err != noErr)
        return err;

    //Empties the buffer, as NumericArrayResize does not
    memset(LStrBuf(**strHandle), '\0', new_length);

    //Fills the string buffer with stringData
    std::copy_n(text.c_str(), new_length, (char*)LStrBuf(**strHandle));

    //Informs the LabVIEW string handle about the size of the size
    LStrLen(**strHandle) = new_length;

    return noErr;
}
