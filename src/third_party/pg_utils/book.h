#ifndef BOOK_H
#define BOOK_H

#include "pg_utils.h"


typedef union{

    uint8 data[16];

    // The struct describes the layout of an entry
    struct{
        uint64 key : 64;
        uint16 move : 16;
        uint16 weight : 16;
        uint32 learn : 32;
    };

} entry_t;


//static void entry_to_file(void *handle, entry_t *entry);
int entry_from_file(void *handle, entry_t *entry);
int find_key(void *handle, uint64 key, entry_t *entry);

#endif
