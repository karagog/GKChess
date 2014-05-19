#ifndef PG_FILE_H
#define PG_FILE_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus



/** Describes a move with its weight. */
typedef struct
{
    char text[6];
    float weight;
} pg_move_t;



/** Opens the file given by the filename.
    \returns A handle for the file, or NULL if the file could not be opened or was an invalid
    polyglot book.  You can get the last error with pg_last_error()
    \param open_mode The file mode.  0 = Read only   1 = Read/Write
*/
void *pg_open_file(char const *filename, int open_mode);

/** Closes the file given by handle. */
void pg_close_file(void *handle);



/** Looks up the position in the book and returns all the moves it found.
    \param handle The handle created by calling open_file()
    \param fen A FEN string to describe the current position
    \param array An array of moves which will be populated with return values,
    whose length is given by the max_array_length parameter.
    In case there are more moves in the book than can be held in the return array,
    the results will be truncated.
    \param max_array_length An input that gives the length of the return array
    \returns The number of items in the return array (can be 0)
*/
unsigned int pg_lookup_moves(void *handle,
                                char const *fen,
                                pg_move_t *array,
                                unsigned int max_array_length);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PG_FILE_H
