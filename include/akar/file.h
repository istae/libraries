#pragma once
#include <stdio.h>
#include <stdlib.h>

using namespace std;

char* getf(char *path)
{
  FILE *f = fopen (path, "rb");
  char *buffer;
  long length;

  if (f) {
    fseek (f, 0, SEEK_END);
    length = ftell (f);
    rewind(f);
    buffer = (char*)malloc(length);
    if (buffer) {
      fread (buffer, 1, length, f);
    }
    fclose (f);
  }
  buffer[length] = 0;
  return buffer;
}
