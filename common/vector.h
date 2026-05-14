#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

typedef struct Vector {
    void*  data;
    size_t elem_size;
    size_t size;
    size_t capacity;
} Vector;

Vector* vectorCreate(size_t elem_size);
void    vectorFree(Vector* vec);
void    vectorPushBack(Vector* vec, const void* elem);
void*   vectorGet(const Vector* vec, size_t index);
size_t  vectorSize(const Vector* vec);
void    vectorClear(Vector* vec);

#endif