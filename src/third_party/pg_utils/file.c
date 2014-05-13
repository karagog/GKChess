#include "file.h"
#include "pg_utils.h"
#include <stdio.h>

PG_EXPORT void *open_file(const char *filename, int om)
{
    char mode[3] = {'r', '\0', '\0'};
    if(om == OpenReadWrite)
        mode[1] = 'w';
    return fopen(filename, mode);
}

PG_EXPORT void close_file(void *h)
{
    fclose((FILE *)h);
}
