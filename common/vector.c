#include "vector.h"

#include <stdlib.h>
#include <string.h>

#define VECTOR_INITIAL_CAPACITY 8

Vector* createVector(size_t elem_size) {
    if (elem_size == 0) return NULL;

    Vector* vector = malloc(sizeof(Vector));
    if (!vector) return NULL;

    vector->data = malloc(elem_size * VECTOR_INITIAL_CAPACITY);
    if (!vector->data) {
        free(vector);
        return NULL;
    }

    vector->size = 0;
    vector->capacity = VECTOR_INITIAL_CAPACITY;
    vector->elem_size = elem_size;

    return vector;
}

void vectorFree(Vector* vector) {
    if (!vector) return;

    free(vector->data);
    free(vector);
}

int vectorReserve(Vector* vector, size_t new_capacity) {
    if (!vector) return 0;
    if (new_capacity <= vector->capacity) return 1;

    void* new_data = realloc(vector->data, new_capacity * vector->elem_size);
    if (!new_data) return 0;

    vector->data = new_data;
    vector->capacity = new_capacity;
    return 1;
}

int appendVectorItem(Vector* vector, const void* item) {
    if (!vector || !item) return 0;

    if (vector->size == vector->capacity) {
        size_t new_capacity = vector->capacity * 2;
        if (!vectorReserve(vector, new_capacity)) return 0;
    }

    char* dst = (char*)vector->data + vector->size * vector->elem_size;
    memcpy(dst, item, vector->elem_size);
    vector->size++;

    return 1;
}

void* getVectorItem(Vector* vector, size_t index) {
    if (!vector || index >= vector->size) return NULL;
    return (char*)vector->data + index * vector->elem_size;
}

const void* getVectorItemConst(const Vector* vector, size_t index) {
    if (!vector || index >= vector->size) return NULL;
    return (const char*)vector->data + index * vector->elem_size;
}