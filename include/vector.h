#pragma once
#include "utilities.h"

typedef struct vector {
    int size;
    int length;
    void* begin;
    void* end;
    void* cap;
} vector;

// x MUST BE AN L-VALUE
void vector_push(vector* v, void* x)
{
   if (v->end == v->cap) {
       v->begin = realloc(v->begin, v->size * v->length * 2);
       v->end = v->begin + (v->size * v->length);
       v->cap = v->begin + (v->size * v->length * 2);
   }

   // copy byte at a time
    // unsigned char *cx = x, *ce = v->end;
    // for (int i = 0; i < v->size; i++)
    //     ce[i] = cx[i];
     memcpy(v->end, x, v->size);

    v->length++;
    v->end += v->size;
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
    memmove(start + v->size, start, v->size * (v->length - pos)); // this is broken fix this!!!!!!!
    memmove(start, x, v->size);
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
   v->end = v->begin;
   v->cap = v->begin + (size*cap);
}
