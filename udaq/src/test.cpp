#include "udaq/types.h"
#include <stdlib.h>
#include <wise_enum/wise_enum.h>

namespace udaq::udaq
{
    WISE_ENUM(Color, (GREEN, 2), RED)

    int test()
    {
        return 1;
    }
}
