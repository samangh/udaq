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

