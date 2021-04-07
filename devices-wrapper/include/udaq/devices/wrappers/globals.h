#pragma once

#ifdef __cplusplus
  #include <cstddef>
  #include <cstdint>
#else
  #include <stdbool.h>
  #include <stddef.h>
  #include <stdint.h>
#endif

// Note: winnt contains a defintion of EXTERN_C_START ad EXTERN_C_END
#ifndef EXTERN_C_START
    #ifdef __cplusplus
        #define EXTERN_C_START extern "C" {
    #else
        #define EXTERN_C_START
    #endif
#endif
#ifndef EXTERN_C_END
    #ifdef __cplusplus
        #define EXTERN_C_END }
    #else
        #define EXTERN_C_END
    #endif
#endif

// Windows an Cygwin have dllexport/dllimport, *nix has visibility
#if defined _WIN32 || defined __CYGWIN__
    #ifdef DEVICES_WRAPPER_BUILDING_DLL
        #ifdef _MSC_VER
            #define DLL_PUBLIC __declspec(dllexport)
        #else
            #define DLL_PUBLIC __attribute__((dllexport))
        #endif
    #else
        #ifdef _MSC_VER
            #define DLL_PUBLIC __declspec(dllimport)
        #else
            #define DLL_PUBLIC __attribute__((dllimport))
        #endif
    #endif
    #define DLL_LOCAL
#else
    #if __GNUC__ >= 4
        #define DLL_PUBLIC __attribute__((visibility("default")))
        #define DLL_LOCAL __attribute__((visibility("hidden")))
    #else
        #define DLL_PUBLIC
        #define DLL_LOCAL
    #endif
#endif
