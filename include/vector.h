#pragma once

#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wpointer-arith"

#ifdef __cplusplus
extern "C" {
#endif

#include <utilities.h>
#include <stdlib.h>
#include <string.h>

void error_exit(char* str)
{
    fprintf(stderr, str);
    exit(1);
}

int safe_realloc(void** p, int size)
{
    void* t = realloc(*p, size);
    if (t) {
        *p = t;
        return 1;
    }
    free(*p);
    return 0;
}
// ERROR_EXIT is a macro defined in utilities.h

typedef struct vector {
    void* begin;
    void* end;
    int size;
    int length;
    int cap;
} vector;

void vector_realloc(vector* v)
{
    if (v->length == v->cap) {
        v->cap *= 2;
        if (!safe_realloc(&v->begin, v->size * v->cap))
            error_exit("vector: realloc failedl\n");
        v->end = v->begin + (v->size * v->length);
    }
}

// x MUST BE AN L-VALUE
void vector_push(vector* v, void* x)
{
    vector_realloc(v);
    memmove(v->end, x, v->size);
    v->end += v->size;
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

void vector_dest(vector* v)
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
   v->end = v->begin;
   v->cap = cap;
}

#ifdef __cplusplus
}
#endif
