#include "file.h"
#include "file_object.h"
#include "pg_utils.h"
#include "book.h"
#include "move.h"
#include "board.h"
#include "hash.h"
#include "error_p.h"
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>


/** Scans the file and returns 1 if it's a valid polyglot database.
 *
 *  Since it's scanning the file anyways, why not also populate the index.
*/
static int validate_file_and_populate_index(file_object_t *);


PG_EXPORT void *pg_open_file(const char *filename, int om)
{
    char mode[] = {'r', '\0', '\0', '\0'};
    int b_ind = 1;
    if(om == 1){
        mode[1] = '+';
        b_ind = 2;
    }
    mode[b_ind] = 'b';


    FILE *f = fopen(filename, mode);
    if(!f){
        set_error_string(strerror(errno));
        return NULL;
    }

    file_object_t *ret = (file_object_t *)malloc(sizeof(file_object_t));
    if(!ret){
        set_error_string("Out of memory");
        fclose(f);
        return NULL;
    }

    // Validate the file
    ret->handle = f;
    if(!validate_file_and_populate_index(ret)){
        // validate_file() sets its own error message
        fclose(f);
        free(ret);
        return NULL;
    }

    set_error_string(0);
    return ret;
}

PG_EXPORT void pg_close_file(void *h)
{
    file_object_t *f = (file_object_t *)h;
    fclose(f->handle);
    free(f);
    set_error_string(0);
}


int validate_file_and_populate_index(file_object_t *f)
{
    long int i, len, entry_cnt;

    // Get the length of the file
    fseek(f->handle, 0L, SEEK_END);
    len = ftell(f->handle);

    if(0 != (0x0F & len)){
        set_error_string("The book size must be a multiple of 16 bytes");
        return 0;
    }

    // Initialize the index
    for(i = 0; i < INDEX_VALUE_COUNT; ++i)
        f->index[i] = -1;

    // Iterate through each entry and make sure the keys are in ascending order,
    //  and populate the index
    entry_cnt = len / POLYGLOT_ENTRY_SIZE;
    uint8 key[POLYGLOT_KEY_SIZE];
    uint8 last_key[POLYGLOT_KEY_SIZE];
    for(i = 0; i < entry_cnt; ++i)
    {
        if(0 != fseek(f->handle, i * POLYGLOT_ENTRY_SIZE, SEEK_SET)){
            set_error_string("Error seeking file");
            return 0;
        }

        if(1 != fread(key, POLYGLOT_KEY_SIZE, 1, f->handle)){
            set_error_string("Error reading file");
            return 0;
        }

        if(0 < i && 0 < memcmp(last_key, key, POLYGLOT_KEY_SIZE)){
            set_error_string("Keys in the book must be in ascending order");
            return 0;
        }

        // Remember the first time we encounter every unique first nibble
        uint8 first_nibble = key[0] >> 4;
        if(-1 == f->index[first_nibble]){
            f->index[first_nibble] = i;
        }

        // Remember the last key
        memcpy(last_key, key, POLYGLOT_KEY_SIZE);
    }

    // Go through the index and fill in any values that are missing
    // Go backwards through the index and fill in empty indices (in case the book is sparse)
    long int last_value = 0;
    for(i = 0; i < INDEX_VALUE_COUNT; ++i){
        if(-1 == f->index[i]){
            f->index[i] = last_value;
        }
        else{
            last_value = f->index[i];
        }
    }

    return 1;
}

