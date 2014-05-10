#ifndef TYPES_H
#define TYPES_H

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

#ifdef _MSC_VER
  typedef unsigned __int64 uint64;
#else
  typedef unsigned long long int uint64;
#endif

#ifdef TRUE
#undef TRUE
#undef FALSE
#endif

#define TRUE  1
#define FALSE 0

#endif


