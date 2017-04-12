#pragma once
#include <stdarg.h>
#include <stdlib.h>

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
int _getline(char* b, FILE* f)
{
    int i, c;
    for (i = 0; (c=getc(f)) != EOF && c != '\n'; i++)
        b[i] = c;
    if (i == 0 && c == '\n')
        b[i++] = '\n';
    b[i] = '\0';
    return i;
}

char* fget(char *path)
{
  FILE *f = fopen (path, "rb");
  char *buffer;
  long length;

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
