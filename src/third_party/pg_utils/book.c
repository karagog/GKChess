#include "pg_utils.h"
#include "book.h"
#include "error_p.h"
#include "file_object.h"
#include "board.h"
#include "hash.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define MAX_MOVES 50

#define OFFSET_MOVE 8
#define OFFSET_WEIGHT 10
#define OFFSET_LEARN 12


/** Converts a big-endian integer in a byte array to the proper uint value. */
static uint64 int64_from_byte_array(uint8 *buf)
{
    return ((uint64)buf[0]) << 56 |
           ((uint64)buf[1]) << 48 |
           ((uint64)buf[2]) << 40 |
           ((uint64)buf[3]) << 32 |
           ((uint64)buf[4]) << 24 |
           ((uint64)buf[5]) << 16 |
           ((uint64)buf[6]) << 8 |
           ((uint64)buf[7]);
}

static void byte_array_from_int64(uint64 i, uint8 *buf)
{
    buf[0] = (i >> 56);
    buf[1] = (i >> 48) & 0x0FF;
    buf[2] = (i >> 40) & 0x0FF;
    buf[3] = (i >> 32) & 0x0FF;
    buf[4] = (i >> 24) & 0x0FF;
    buf[5] = (i >> 16) & 0x0FF;
    buf[6] = (i >> 8) & 0x0FF;
    buf[7] = (i) & 0x0FF;
}

//static uint32 int32_from_byte_array(uint8 *buf)
//{
//    return ((uint32)buf[0]) << 24 |
//           ((uint32)buf[1]) << 16 |
//           ((uint32)buf[2]) << 8 |
//           ((uint32)buf[3]);
//}

//static void byte_array_from_int32(uint32 i, uint8 *buf)
//{
//    buf[1] = (i >> 24) & 0x0FF;
//    buf[2] = (i >> 16) & 0x0FF;
//    buf[3] = (i >> 8) & 0x0FF;
//    buf[4] = (i) & 0x0FF;
//}

static uint16 int16_from_byte_array(uint8 *buf)
{
    return ((uint16)buf[0]) << 8 |
           ((uint16)buf[1]);
}

//static void byte_array_from_int16(uint16 i, uint8 *buf)
//{
//    buf[0] = (i >> 8) & 0x0FF;
//    buf[1] = (i) & 0x0FF;
//}


typedef struct{

    // The data for this entry
    uint8 data[16];

    // The index of the entry in the file. Multiply this number by the size of
    //  the data array to get the byte offset in the file.
    long int offset;
}
entry_t;

static const entry_t entry_none = {{0}, 0};


// Here are the accessor methods for the entry data.  In c++ these would be class methods

static uint64 get_key(entry_t *e){
    return int64_from_byte_array(e->data);
}
static void set_key(entry_t *e, uint64 key){
    byte_array_from_int64(key, e->data);
}

static uint16 get_move(entry_t *e){
    return int16_from_byte_array(&e->data[OFFSET_MOVE]);
}
//static void set_move(entry_t *e, uint16 mv){
//    byte_array_from_int16(mv, &e->data[OFFSET_MOVE]);
//}

static uint16 get_weight(entry_t *e){
    return int16_from_byte_array(&e->data[OFFSET_WEIGHT]);
}
//static void set_weight(entry_t *e, uint16 wt){
//    byte_array_from_int16(wt, &e->data[OFFSET_WEIGHT]);
//}

//static uint32 get_learn(entry_t *e){
//    return int32_from_byte_array(&e->data[OFFSET_LEARN]);
//}
//static void set_learn(entry_t *e, uint32 learn){
//    byte_array_from_int32(learn, &e->data[OFFSET_LEARN]);
//}


entry_t entry_from_file(FILE *h)
{
    entry_t ret;
    if(fread(ret.data, POLYGLOT_ENTRY_SIZE, 1, h) != 1){
        // If this failed, it's because we're at the EOF
        ret = entry_none;
    }
    return ret;
}

// Finds the first instance of the key in the book.
//  Returns a null entry on failure (check key against 0)
entry_t find_entry(file_object_t *f, uint64 key)
{
    entry_t ret = entry_none;
    long int first, last, middle;
    uint64 last_key, middle_key;
    uint8 tmpbuf[8];

    first = -1;
    if(0 == fseek(f->handle, -POLYGLOT_ENTRY_SIZE, SEEK_END))
    {
        last = ftell(f->handle)/POLYGLOT_ENTRY_SIZE;

        // Only read in the key at this time, not the full entry
        fread(tmpbuf, POLYGLOT_KEY_SIZE, 1, f->handle);
        last_key = int64_from_byte_array(tmpbuf);

        // Do a binary search to find the first entry with matching key
        while(1)
        {
            if(last-first==1)
            {
                if(last_key == key)
                {
                    // Found it...

                    // Set the key and offset
                    set_key(&ret, last_key);
                    ret.offset = last;

                    // Read the rest of the data for the entry (move, weight, learn)
                    fread(&ret.data[8], POLYGLOT_ENTRY_SIZE - POLYGLOT_KEY_SIZE, 1, f->handle);
                }
                break;
            }

            // Cut the search area in half and test the key in the middle
            middle=(first+last)/2;
            fseek(f->handle, POLYGLOT_ENTRY_SIZE*middle, SEEK_SET);
            fread(tmpbuf, POLYGLOT_KEY_SIZE, 1, f->handle);
            middle_key = int64_from_byte_array(tmpbuf);

            // If the key is to the left of the middle, move the last pointer and try again
            if(key <= middle_key){
                last=middle;
                last_key=middle_key;
            }

            // Else if the key is to the right of the middle, move the first pointer and try again
            else{
                first=middle;
            }
        }
    }
    return ret;
}

PG_EXPORT uint64 pg_compute_key(char const *fen)
{
    uint64 ret = 0;
    board_t board;
    if(0 == board_from_fen(&board,fen))
        ret = hash(&board);
    set_error_string(0 == ret ? 0 : "Invalid FEN");
    return ret;
}


PG_EXPORT unsigned int pg_lookup_moves(void *f, uint64 key, pg_move_t *array, unsigned int max_array_length)
{
    file_object_t *fo = (file_object_t *)f;
    int ret_length = 0;
    unsigned int i;
    entry_t entry = find_entry(fo, key);
    if(key == get_key(&entry))
    {
        entry_t entries[MAX_MOVES];
        entries[0]=entry;
        unsigned int count=0;
        fseek(fo->handle, POLYGLOT_ENTRY_SIZE*entry.offset, SEEK_SET);
        while(1){
            entry = entry_from_file(fo->handle);
            uint64 cur_key = get_key(&entry);
            if(cur_key != key){
                break;
            }
            if(count==MAX_MOVES){
                break;
            }
            entries[count++] = entry;
        }

        int total_weight=0;
        for(i = 0;i<count;i++)
            total_weight += get_weight(&entries[i]);

        pg_move_t *cur_move = array;
        entry_t *cur_entry = entries;
        for(i = 0; i < count && i < max_array_length; ++i, ++cur_move, ++cur_entry)
        {
            uint16 move_data = get_move(cur_entry);
            cur_move->dest_col = 0x7 & move_data;
            cur_move->dest_row = 0x7 & (move_data >> 3);
            cur_move->source_col = 0x7 & (move_data >> 6);
            cur_move->source_row = 0x7 & (move_data >> 9);
            cur_move->promoted_piece = 0x7 & (move_data >> 12);
            cur_move->weight = (float) get_weight(cur_entry) / total_weight * 100;
        }

        ret_length = count;
    }
    set_error_string(0);
    return ret_length;
}

PG_EXPORT void pg_move_to_string(pg_move_t *m, char *s)
{
    s[0] = 'a' + m->source_col;
    s[1] = '1' + m->source_row;
    s[2] = 'a' + m->dest_col;
    s[3] = '1' + m->dest_row;

    if(m->promoted_piece == promote_none)
        s[4] = '\0';
    else{
        char c;
        switch(m->promoted_piece)
        {
        case promote_knight:
            c = 'n';
            break;
        case promote_bishop:
            c = 'b';
            break;
        case promote_rook:
            c = 'r';
            break;
        case promote_queen:
            c = 'q';
            break;
        default:
            // Don't crash here, just ignore the invalid promoted piece
            c = '\0';
            break;
        }
        s[4] = c;
        s[5] = '\0';
    }
}
