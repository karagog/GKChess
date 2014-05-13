#ifndef PG_FILE_H
#define PG_FILE_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


enum OpenModeEnum
{
    OpenRead = 0,
    OpenReadWrite = 1
};

/** Opens the file given by the filename. 
    \returns A handle for the file, or NULL if the file could not be opened
*/
void *open_file(char const *filename, int open_mode);

/** Closes the file given by handle. */
void close_file(void *handle);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PG_FILE_H
