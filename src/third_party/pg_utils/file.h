#ifndef PG_FILE_H
#define PG_FILE_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/** Opens the file given by the filename.
    \returns A handle for the file, or NULL if the file could not be opened
    \param open_mode The file mode.  0 = Read only   1 = Read/Write
*/
void *pg_open_file(char const *filename, int open_mode);

/** Closes the file given by handle. */
void pg_close_file(void *handle);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PG_FILE_H
