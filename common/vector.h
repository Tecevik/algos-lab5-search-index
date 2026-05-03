#pragma once

#include <stddef.h>

typedef struct {
    void*  data;
    size_t size;
    size_t capacity;
    size_t elem_size;
} Vector;

Vector* createVector(size_t elem_size);
void    vectorFree(Vector* vector);

int     appendVectorItem(Vector* vector, const void* item);
void*   getVectorItem(Vector* vector, size_t index);
const void* getVectorItemConst(const Vector* vector, size_t index);

int     vectorReserve(Vector* vector, size_t new_capacity);