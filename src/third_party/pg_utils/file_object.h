#ifndef PG_FILEOBJECT_H
#define PG_FILEOBJECT_H

#include <stdio.h>

#define INDEX_VALUE_COUNT 16


/** Let's make this a struct so we can expand it if necessary. */
typedef struct
{
    FILE *handle;
}
file_object_t;


#endif // PG_FILEOBJECT_H
