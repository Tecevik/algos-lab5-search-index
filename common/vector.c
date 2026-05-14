#include "vector.h"
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 4

Vector* vectorCreate(size_t elem_size) {
    Vector* vec = (Vector*)malloc(sizeof(Vector));
    if (!vec) return NULL;
    
    vec->elem_size = elem_size;
    vec->size = 0;
    vec->capacity = INITIAL_CAPACITY;
    vec->data = malloc(elem_size * INITIAL_CAPACITY);
    
    if (!vec->data) {
        free(vec);
        return NULL;
    }
    
    return vec;
}

void vectorFree(Vector* vec) {
    if (!vec) return;
    free(vec->data);
    free(vec);
}

void vectorPushBack(Vector* vec, const void* elem) {
    if (!vec || !elem) return;
    
    if (vec->size >= vec->capacity) {
        size_t new_capacity = vec->capacity * 2;
        void* new_data = realloc(vec->data, vec->elem_size * new_capacity);
        if (!new_data) return;
        
        vec->data = new_data;
        vec->capacity = new_capacity;
    }
    
    memcpy((char*)vec->data + vec->size * vec->elem_size, elem, vec->elem_size);
    vec->size++;
}

void* vectorGet(const Vector* vec, size_t index) {
    if (!vec || index >= vec->size) return NULL;
    return (char*)vec->data + index * vec->elem_size;
}

size_t vectorSize(const Vector* vec) {
    return vec ? vec->size : 0;
}

void vectorClear(Vector* vec) {
    if (!vec) return;
    vec->size = 0;
}