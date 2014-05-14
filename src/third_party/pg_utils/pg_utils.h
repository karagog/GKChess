#ifndef PG_UTILS_H
#define PG_UTILS_H


/** \file pg_utils.h
 *
 *  The main include file for the Polyglot utils library for chess opening books.
 *
 *  This is a very simple C library that at this point only provides a way of reading
 *  polyglot books.  It does not do validation, so if used on a non-polyglot book it
 *  will probably crash your application.  If any library function is given invalid inputs
 *  its behavior is undefined.
 *
 *  The basic api is like this, to look up a position given in FEN:
 *
 *  // Open a valid book
 *  void *file_handle = pg_open_file("valid_polyglot_book.bin", 0);
 *  if(!file_handle)
 *      return 1;   // error
 *
 *  unsigned int length;
 *  pg_move_t *move_array = pg_lookup_moves(file_handle, fen, &length);
 *  for(int i = 0; i < length; ++i)
 *      move_array[i];  // Access all the moves
 *
 *  // Clean up the memory used for the move array
 *  pg_cleanup_moves(move_array);
 *
 *  // Be sure to close the file when you're done
 *  pg_close_file(file_handle);
*/



typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

#ifdef _MSC_VER
  typedef unsigned __int64 uint64;
#else
  typedef unsigned long long int uint64;
#endif



// Some stuff to expose certain functions of ours

#if defined(WIN32)
    // Windows
    #define EXPORT __declspec(dllexport)
    #define IMPORT __declspec(dllimport)
#else
    // Linux
    #define EXPORT __attribute__((visibility("default")))
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


