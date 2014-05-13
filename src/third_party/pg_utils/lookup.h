#ifndef PG_LOOKUP_H
#define PG_LOOKUP_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/** Describes a move with its weight. */
typedef struct
{
    char text[6];
    float weight;
} move_t;

/** Looks up the position in the book and returns all the moves it found.
    \returns An array of moves, whose length is given by the ret_length parameter
    \param file_handle The handle created by calling open_file()
    \param fen A FEN string to describe the current position
    \param ret_length An output that gives the length of the return array (can be 0)
*/
move_t *lookup_moves(void *file_handle, char const *fen, unsigned int *ret_length);

/** Cleans up the array of moves returned by lookup_moves. */
void cleanup_moves(move_t *);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif
