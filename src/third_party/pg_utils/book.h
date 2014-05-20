#ifndef BOOK_H
#define BOOK_H

#include "pg_utils.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/** The size of one polyglot entry, in bytes. */
#define POLYGLOT_ENTRY_SIZE 16

/** The size of a polyglot entry key, in bytes. */
#define POLYGLOT_KEY_SIZE 8


/** These are the enumerations given in the Polyglot spec. */
enum promoted_piece_enum
{
    promote_none = 0,
    promote_knight = 1,
    promote_bishop = 2,
    promote_rook = 3,
    promote_queen = 4
};


/** Describes a move with its weight and app-specific 'learn' value. */
typedef struct
{
    // 0-based indices describe the source and dest squares
    uint8 source_col, source_row;
    uint8 dest_col, dest_row;
    
    // The value of this corresponds to the promoted_piece_enum
    uint8 promoted_piece;
    
    // The weight is between 0 and 100%
    float weight;
    
    // The learn value only means something to the application that's using it
    uint32 learn;
} pg_move_t;


/** Returns the polyglot hash for the given position. 
    \param fen The position of the board in FEN
    \returns The position hash, or 0 if there was a problem (invalid FEN)
*/
uint64 pg_compute_key(char const *fen);


/** Looks up the position in the book and returns all the moves it found.
    \param handle The handle created by calling open_file()
    \param key The position key, acquired from pg_compute_key()
    \param array An array of moves which will be populated with return values,
    whose length is given by the max_array_length parameter.
    In case there are more moves in the book than can be held in the return array,
    the results will be truncated.
    \param max_array_length An input that gives the length of the return array
    \returns The number of items in the return array (can be 0)
*/
unsigned int pg_lookup_moves(void *handle,
                                uint64 key,
                                pg_move_t *array,
                                unsigned int max_array_length);

                                
/** Converts the move to a string.
    \param s An array of memory to fill with the string.  It must be
    at least length 6 (4 for move text, 1 for promoted piece, 1 for null terminator)
*/
void pg_move_to_string(pg_move_t *, char *s);


#ifdef __cplusplus
}
#endif // __cplusplus                                

#endif
