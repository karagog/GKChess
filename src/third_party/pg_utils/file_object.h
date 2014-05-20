#ifndef PG_FILEOBJECT_H
#define PG_FILEOBJECT_H

#include <stdio.h>

#define INDEX_VALUE_COUNT 16


typedef struct
{
    FILE *handle;
    long int index[INDEX_VALUE_COUNT];
}
file_object_t;


#endif // PG_FILEOBJECT_H
