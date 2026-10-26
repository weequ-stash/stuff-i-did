#include "vector.h"
#include <stdlib.h>
#include <string.h>

vector* vector_init(size_t capacity) {
    vector* vec = malloc(sizeof(vector));
    if(!vec) return NULL;

    vec->_capacity = capacity;
    vec->size = 0;
    vec->data = malloc(capacity * sizeof(char*));
    if(!vec->data) {
        free(vec);
        return NULL;
    }

    return vec;
}

vector* vector_realloc(vector* vec, size_t new_capacity) {
    vec->_capacity = new_capacity;

    char** temp = realloc(vec->data, vec->_capacity * sizeof(char*));
    if(!temp) {
        for(size_t i = 0; i < vec->size; ++i) {
            free(vec->data[i]);
        }
        free(vec->data);
        free(vec);
        return NULL;
    }

    vec->data = temp;
    return vec;
}

void vector_push(vector* vec, const char* str) {
    if(vec->size == vec->_capacity) {
        vec->_capacity <<= 1;
        char** temp = realloc(vec->data, vec->_capacity * sizeof(char*));
        if(!temp) {
            for(size_t i = 0; i < vec->size; ++i) {
                free(vec->data[i]);
            }
            free(vec->data);
            free(vec);
            return;
        }

        vec->data = temp;
    }
    vec->data[vec->size] = malloc((strlen(str) + 1) * sizeof(char));
    if(!vec->data[vec->size]) {
        for(size_t i = 0; i < vec->size; ++i) {
            free(vec->data[i]);
        }
        free(vec->data);
        free(vec);
        return;
    }

    strcpy(vec->data[vec->size], str);
    ++vec->size;
}

void vector_delete(vector* vec) {
    if(!vec) return;

    for(size_t i = 0; i < vec->size; ++i) {
        free(vec->data[i]);
    }
    free(vec->data);
    free(vec);
}