#include "file.h"
#include "pg_utils.h"
#include <stdio.h>

PG_EXPORT void *pg_open_file(const char *filename, int om)
{
    char mode[3] = {'r', '\0', '\0'};
    if(om == 1)
        mode[1] = 'w';
    return fopen(filename, mode);
}

PG_EXPORT void pg_close_file(void *h)
{
    fclose((FILE *)h);
}
