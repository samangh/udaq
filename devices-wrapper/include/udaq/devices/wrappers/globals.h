#pragma once


#ifdef __cplusplus
#include <cstddef>
  #define EXTERN_C extern "C"
#else
  #include <stdbool.h>
  #include <stddef.h>
  #define EXTERN_C
#endif

#if defined _WIN32 || defined __CYGWIN__
  #ifdef DEVICES_WRAPPER_BUILDING_DLL
    #ifdef __GNUC__
     #define DLL_PUBLIC EXTERN_C __attribute__((dllexport))
    #else
      #define DLL_PUBLIC __declspec(dllexport)
    #endif
  #else
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__ ((dllimport))
    #else
      #define DLL_PUBLIC __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #endif
#else
    #define DLL_PUBLIC
#endif

