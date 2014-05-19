#include "file.h"
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

#define MAX_MOVES 50
#define INDEX_VALUES 16

/** The size of one polyglot entry, in bytes. */
#define POLYGLOT_ENTRY_SIZE 16

/** The size of a polyglot entry key, in bytes. */
#define POLYGLOT_KEY_SIZE 8

struct file_handle_t
{
    FILE *handle;
    long int index[INDEX_VALUES];
};

/** Scans the file and returns true if it's a valid polyglot database.
 *
 *  Since it's scanning the file anyways, why not also populate the index.
*/
static bool validate_file_and_populate_index(file_handle_t &);


PG_EXPORT unsigned int pg_lookup_moves(void *f, char const *fen, pg_move_t *array, unsigned int max_array_length)
{
    board_t board;
    int ret_length = 0;

    if(0 == board_from_fen(&board,fen))
    {
        unsigned int i;
        uint64 key=hash(&board);
        entry_t entry;
        int offset=find_key(f,key,&entry);
        if(entry.key == key)
        {
            entry_t entries[MAX_MOVES];
            entries[0]=entry;
            unsigned int count=1;
            fseek((FILE*)f,16*(offset+1),SEEK_SET);
            while(1){
                if(entry_from_file(f,&entry)){
                    break;
                }
                if(entry.key!=key){
                    break;
                }
                if(count==MAX_MOVES){
                    break;
                }
                entries[count++]=entry;
            }

            int total_weight=0;
            for(i = 0;i<count;i++)
                total_weight+=entries[i].weight;

            pg_move_t *cur_move = array;
            entry_t *cur_entry = entries;
            for(i = 0; i < count && i < max_array_length; ++i, ++cur_move, ++cur_entry)
            {
                move_to_string(cur_move->text, cur_entry->move);
                cur_move->weight = (float) cur_entry->weight / total_weight * 100;
            }

            ret_length = count;
        }
    }
    set_error_string(0);
    return ret_length;
}


PG_EXPORT void *pg_open_file(const char *filename, int om)
{
    char mode[3] = {'r', '\0', '\0'};
    if(om == 1)
        mode[1] = 'w';

    File *f = fopen(filename, mode);
    if(!f){
        set_error_string(strerror(errno));
        return NULL;
    }

    file_handle_t *ret = (file_handle_t)malloc(sizeof(file_handle_t));
    if(!ret){
        set_error_string("Out of memory");
        fclose(f);
        return NULL;
    }

    // Validate the file
    ret->handle = f;
    if(!validate_file_and_populate_index(*ret)){
        // validate_file() sets its own error message
        fclose(f);
        return NULL;
    }

    set_error_string(0);
    return ret;
}

PG_EXPORT void pg_close_file(void *h)
{
    file_handle_t *f = (file_handle_t)h;
    fclose(f->handle);
    free(f);
    set_error_string(0);
}


bool validate_file_and_populate_index(file_handle_t &f)
{
    long int i, len, entry_cnt;

    // Get the length of the file
    fseek(f.handle, 0L, SEEK_END);
    len = ftell(f.handle);

    if(0 != (0x0F && len)){
        set_error_string("The book size must be a multiple of 16 bytes");
        return false;
    }

    // Initialize the index
    for(i = 0; i < INDEX_VALUES; ++i)
        f.index[i] = -1;

    // Iterate through each entry and make sure the keys are in ascending order,
    //  and populate the index
    entry_cnt = len / POLYGLOT_ENTRY_SIZE;
    uint8 key[POLYGLOT_KEY_SIZE];
    uint8 last_key[POLYGLOT_KEY_SIZE];
    for(i = 0; i < entry_cnt; ++i)
    {
        fseek(f.handle, i * POLYGLOT_ENTRY_SIZE, SEEK_SET);
        if(fread(key, POLYGLOT_KEY_SIZE, 1, f.handle) != POLYGLOT_KEY_SIZE){
            set_error_string(strerror(errno));
            return false;
        }

        if(0 < i && 0 < memcmp(last_key, key, POLYGLOT_KEY_SIZE)){
            set_error_string("Keys in the book must be in ascending order");
            return false;
        }

        // Remember the first time we encounter every unique first nibble
        uint8 first_nibble = key[0] >> 4;
        if(-1 == f.index[first_nibble]){
            f.index[first_nibble] = i;
        }

        // Remember the last key
        memcpy(last_key, key, POLYGLOT_KEY_SIZE);
    }

    // Go through the index and fill in any values that are missing
    // Go backwards through the index and fill in empty indices (in case the book is sparse)
    long int last_value = 0;
    for(i = 0; i < INDEX_VALUES; ++i){
        if(-1 == f.index[i]){
            f.index[i] = last_value;
        }
        else{
            last_value = f.index[i];
        }
    }

    return true;
}

