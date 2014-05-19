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
} pg_move_t;


/** Looks up the position in the book and returns all the moves it found.
    \param file_handle The handle created by calling open_file()
    \param fen A FEN string to describe the current position
    \param array An array of moves which will be populated with return values,
    whose length is given by the max_array_length parameter.
    In case there are more moves in the book than can be held in the return array,
    the results will be truncated.
    \param max_array_length An input that gives the length of the return array
    \returns The number of items in the return array (can be 0)
*/
unsigned int pg_lookup_moves(void *file_handle,
                                char const *fen,
                                pg_move_t *array,
                                unsigned int max_array_length);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif
