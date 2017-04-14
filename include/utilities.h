#pragma once
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

// returns lower bound on SORTED array

void debug(char* str, ...)
{
    if (DEBUG == 0)
        return;
    va_list args;
    va_start(args, str);
    vprintf(str, args);
    va_end(args);
}

// just reads input from stdin
int _getline(char* b, int max, FILE* f)
{
    int i, c;
    for (i = 0; i < max && (c=getc(f)) != EOF && c != '\n'; i++)
        b[i] = c;
    if (i == 0 && c == '\n')
        b[i++] = '\n';
    b[i] = '\0';
    return i;
}

char* fget(char *path)
{
  FILE *f = fopen (path, "rb");
  char *buffer = NULL;
  long length = 0;

  if (f) {
    fseek (f, 0, SEEK_END);
    length = ftell (f);
    rewind(f);
    buffer = malloc(length);
    if (buffer) {
      fread (buffer, 1, length, f);
    }
    fclose (f);
  }
  buffer[length] = '\0';
  return buffer;
}

void freelist(int len, ...)
{
    va_list args;
    va_start(args, len);

    for (int i=0; i < len; i++) {
        void* p = va_arg(args, void*);
        free(p);
    }
    va_end(args);
}

void _swap(void* a, void* b, const size_t size)
{
    //swap byte at a time
    unsigned char *p = a, *q = b, tmp;
    for (size_t i = 0; i != size; ++i) {
        tmp = p[i];
        p[i] = q[i];
        q[i] = tmp;
    }
}

// partitions the array and returns the index of the first item that returns true from the CMP function
int partition(void* a, size_t len, const size_t size, int (*cmp)(void*))
{
    if (len == 0)
        return -1;

    for(int i=0; ;i++) {
        if (!cmp(a+(i*size))) {
            do {
                --len;
                if (len < i)
                    return len+1;
            }
            while (!cmp(a+(len*size)));
            _swap(a+(i*size), a+(len*size), size);
        }
    }
}

// 1) array 2) array length 3) item to find 4) item size
int lower_bound(void* n, const int len, void* m, const int size)
{
    int b = 0;
    int e = len-1;
    int mid = (e + b) / 2;
    while (b <= e) {
        int cmp = memcmp(n+(mid*size), m, size);
        if (cmp < 0)
            b = mid+1;
        else if (cmp > 0)
            e = mid - 1;
        else
            return mid;
        mid = (e + b) / 2;
    }
    return b;
}
