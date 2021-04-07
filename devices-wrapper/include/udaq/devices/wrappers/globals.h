#pragma once

#ifdef __cplusplus
  #include <cstddef>
#else
  #include <stdbool.h>
  #include <stddef.h>
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


#ifdef DEVICES_WRAPPER_BUILDING_DLL
  #ifdef _MSC_VER
    #define DLL_PUBLIC __declspec(dllexport)
  #else
    #define DLL_PUBLIC EXTERN_C __attribute__((dllexport))
  #endif
#else
  #ifdef _MSC_VER
    #define DLL_PUBLIC __declspec(dllimport)
  #else
    #define DLL_PUBLIC __attribute__ ((dllimport))
  #endif
#endif

