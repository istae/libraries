#pragma once

#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wpointer-arith"

#ifdef __cplusplus
extern "C" {
#endif

#include <utilities.h>
#include <stdlib.h>
#include <string.h>

typedef struct vector {
    void* begin;
    int size;
    int length;
    int cap;
} vector;

void  vector_realloc(vector* v)
{
    if (v->length == v->cap) {
        v->cap *= 2;
        if (!safe_realloc(&v->begin, v->cap * v->size))
            error_exit("vector: realloc failed after %.2lf mb !", (double)v->cap * v->size / 1024 / 1024 );
    }
}

// x MUST BE AN L-VALUE
void vector_push(vector* v, void* x)
{
    vector_realloc(v);
    memcpy(v->begin + (v->size * v->length), x, v->size);
    v->length++;
}

void* vector_index(vector* v, int pos)
{
   return v->begin + pos * v->size;
}

// 1) vector pointer 2) object to insert, 3) insert position
void vector_insert(vector* v, void* x, int pos)
{
    vector_push(v, x);
    if (pos >= v->length)
        return;

    void* start = vector_index(v, pos);
    memcpy(start + v->size, start, v->size * (v->length - pos - 1));
    memcpy(start, x, v->size);
}

void vector_insert_int(vector* v,int x, int pos)
{
    vector_push(v, &x);
    if (pos >= v->length)
        return;

    int* start = (int*)vector_index(v, pos);
    memcpy(start+1, start, (v->length - pos - 1)*sizeof(int));
    memcpy(start, &x, v->size);
}

void vector_free(vector* v)
{
   free(v->begin);
}

// users better give a c
void vector_init(vector* v, int size, size_t cap)
{
   if (cap <= 0)
       cap = 1;

   v->length = 0;
   v->size = size;
   v->begin = malloc(size*cap);
   if (v->begin == NULL) error_exit("vector: init malloc failedl\n");
   v->cap = cap;
}

#ifdef __cplusplus
}
#endif
