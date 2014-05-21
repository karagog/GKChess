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

    ret->handle = f;
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


PG_EXPORT int pg_validate_file(void *h, void (*progress_cb)(int))
{
    file_object_t *f = (file_object_t *)h;
    long int i, len, entry_cnt;
    long int progress_inc, progress_cnt = 0;
    int progress = 0;
    uint8 key[POLYGLOT_KEY_SIZE];
    uint8 last_key[POLYGLOT_KEY_SIZE];

    // Get the length of the file
    fseek(f->handle, 0L, SEEK_END);
    len = ftell(f->handle);

    if(0 != (0x0F & len)){
        set_error_string("The book size must be a multiple of 16 bytes");
        return 0;
    }
    
    // Iterate through each entry and make sure the keys are in ascending order,
    //  and populate the index
    entry_cnt = len / POLYGLOT_ENTRY_SIZE;
    
    // Only update every so often
    progress_inc = entry_cnt / 100;
    
    for(i = 0; i < entry_cnt; ++i)
    {
        if(0 != fseek(f->handle, i * POLYGLOT_ENTRY_SIZE, SEEK_SET)){
            set_error_string("Error seeking file");
            return 1;
        }

        if(1 != fread(key, POLYGLOT_KEY_SIZE, 1, f->handle)){
            set_error_string("Error reading file");
            return 1;
        }

        if(0 < i && 0 < memcmp(last_key, key, POLYGLOT_KEY_SIZE)){
            set_error_string("Keys in the book must be in ascending order");
            return 1;
        }

        // Remember the last key
        memcpy(last_key, key, POLYGLOT_KEY_SIZE);
        
        if(progress_cb){
            // Notify of progress update
            ++progress_cnt;
            if(progress_cnt == progress_inc){
                progress_cnt = 0;
                progress_cb(++progress);
            }
        }
    }
    return 0;
}

