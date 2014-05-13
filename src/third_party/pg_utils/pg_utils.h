#ifndef PG_UTILS_H
#define PG_UTILS_H

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

#ifdef _MSC_VER
  typedef unsigned __int64 uint64;
#else
  typedef unsigned long long int uint64;
#endif



// Some stuff to expose certain functions of ours

#if defined(_MSC_VER)
    //  Microsoft
    #define EXPORT __declspec(dllexport)
    #define IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
    //  GCC
    #define EXPORT __attribute__((visibility("default")))
    #define IMPORT
#else
    #define EXPORT
    #define IMPORT
#endif


#ifdef DLL_EXPORT
    #define PG_EXPORT  EXPORT
#else
    #define PG_EXPORT  IMPORT
#endif


#include "file.h"
#include "lookup.h"


#endif


