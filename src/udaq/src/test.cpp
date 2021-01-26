#include "udaq.h"
#include <stdlib.h>

__declspec(dllexport) int test(void *t) {
    int *i = (int*)t;
    return *i;
}

__declspec(dllexport) void * init() {
    int *i = (int*) malloc(sizeof(int));
    *i = 365;
    return (void *)i;
}


namespace udaq::udaq
{
    int test()
    {
        return 1;
    }
}
