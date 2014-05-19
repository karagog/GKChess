#include "lookup.h"
#include "book.h"
#include "move.h"
#include "board.h"
#include "hash.h"
#include <malloc.h>
#include <stdio.h>
#include <assert.h>

#define MAX_MOVES 50

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
    return ret_length;
}
