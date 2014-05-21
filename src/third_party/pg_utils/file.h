#ifndef PG_FILE_H
#define PG_FILE_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus



/** Opens the file given by the filename.
    This does not validate the file.  To do that, call pg_validate_file().
    \returns A handle for the file, or NULL if the file could not be opened or was an invalid
    polyglot book.  You can get the last error with pg_last_error()
    \param open_mode The file mode.  0 = Read only   1 = Read/Write
*/
void *pg_open_file(char const *filename, int open_mode);

/** Closes the file given by handle. */
void pg_close_file(void *handle);

/** Validates the file and returns 0 if it is a valid polyglot book.  This
    operation can take a while on big books, so you can pass a callback
    function to receive progress updates.
    
    \param handle The file handle returned by pg_open_file()
    \param on_progress_update An optional callback function to receive progress
    notifications, since this can be a long operation.  Pass 0 to ignore updates.
    \returns 0 if a valid PG book, 1 otherwise (in which case the error string
    will be set)
*/
int pg_validate_file(void *handle, void (*on_progress_update)(int));



#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PG_FILE_H
