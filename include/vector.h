#pragma once
#include "utilities.h"
#include <assert.h>
typedef struct vector {
    int size;
    int length;
    void* begin;
    void* end;
    void* cap;
} vector;

void vector_realloc(vector* v)
{
    if (v->end == v->cap) {
        void* tmp = realloc(v->begin, v->size * v->length * 2);
        if (tmp == NULL) {
            fprintf(stderr, "vector: realloc failed\n");
            free(v->begin);
            exit(1);
        }
        v->begin = tmp;
        v->end = v->begin + (v->size * v->length);
        v->cap = v->begin + (v->size * v->length * 2);
    }
}

// x MUST BE AN L-VALUE
void vector_push(vector* v, void* x)
{
    vector_realloc(v);
    memcpy(v->end, x, v->size);
    v->end += v->size;
    v->length++;
}

void* vector_index(vector* v, int x)
{
   return v->begin + x * v->size;
}

// 1) vector pointer 2) object to insert, 3) insert position
void vector_insert(vector* v, void* x, int pos)
{
    vector_push(v, x);
    if (pos >= v->length)
        return;

    void* start = vector_index(v, pos);
    memmove(start + v->size, start, v->size * (v->length - pos - 1)); // this is broken fix this!!!!!!!
    memcpy(start, x, v->size);
}

void vector_insert_int(vector* v,int x, int pos)
{
    vector_realloc(v);
    if (pos >= v->length)
        return;

    int* start = vector_index(v, pos);
    memcpy(start+1, start, (v->length - pos - 1)*sizeof(int));
    memcpy(start, &x, v->size);
}

void vector_dest(vector* v)
{
   free(v->begin);
}

void vector_init(vector* v, int size, size_t cap)
{
   if (cap <= 0)
       cap = 1;

   v->length = 0;
   v->size = size;
   v->begin = malloc(size*cap);
   if (v->begin == NULL) {
       fprintf(stderr, "vector: init malloc failedl\n");
       exit(1);
   }
   v->end = v->begin;
   v->cap = v->begin + (size*cap);
}
