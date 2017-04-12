#pragma once

typedef struct vector {
   size_t length;      // vector length
   int size;        // type size
   size_t capacity;
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
       v->capacity = v->size * v->length * 2;
   }

   // copy byte at a time
   unsigned char *cx = x, *ce = v->end;
   for (int i = 0; i < v->size; i++)
       ce[i] = cx[i];

   v->length++;
   v->end += v->size;
}

void* vector_index(vector* v, size_t x)
{
   return v->begin + x * v->size;
}

void vector_dest(vector* v)
{
   free(v->begin);
}

// initial vector capacity is len
void vector_init(vector* v, int size, size_t cap)
{
   if (cap <= 0)
       cap = 1;

   v->length = 0;
   v->size = size;
   v->capacity = cap;

   v->begin = malloc(size*cap);
   v->end = v->begin;
   v->cap = v->begin + (size*cap);
}
