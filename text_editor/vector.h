#ifndef VECTOR_H
#define VECTOR_H
#include <stddef.h>

typedef struct {
    size_t size;
    size_t _capacity;
    char** data;
} vector;

vector* vector_init(size_t capacity);
vector* vector_relloc(vector* vec, size_t new_capacity);
void vector_push(vector* vec, const char* str);
void vector_delete(vector* vec);
#endif