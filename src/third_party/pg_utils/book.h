#ifndef BOOK_H
#define BOOK_H

#include "pg_utils.h"

typedef struct {
    uint64 key;
    uint16 move;
    uint16 weight;
    uint32 learn;
} entry_t;

//static void entry_to_file(void *handle, entry_t *entry);
int entry_from_file(void *handle, entry_t *entry);
int find_key(void *handle, uint64 key, entry_t *entry);

#endif
