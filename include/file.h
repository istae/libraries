#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>

int isdir(const char* fname)
{
    struct stat st;
    stat(fname, &st); // error check laer
    return ((st.st_mode & S_IFMT) == S_IFDIR);
}

int fset(char* path, void* buffer, int len)
{
    FILE* f = fopen(path, "wb");
    if (!f)
        return 0;
    fwrite (buffer, 1, len, f);
    fclose (f);
    return 1;
}

char* fget(char* path, int* len)
{
    char* buffer = NULL;
    FILE *f = fopen (path, "rb");
    if (f) {
        fseek (f, 0, SEEK_END);
        int length = ftell(f);
        fseek (f, 0, SEEK_SET);
        buffer = (char*)malloc(length+1); // + '\0'
        if (buffer != NULL) {
            fread (buffer, 1, length, f);
            buffer[length] = '\0';
            *len = length;
        }
        fclose (f);
    }
    return buffer;
}

int fcat(char* f1, char* f2)
{
    int len;
    char* b = fget(f2, &len);
    int r = fset(f1, b, len);
    free(b);
    return r;
}

time_t fmtime(char* fname)
{
    struct stat st;
    stat(fname, &st);
    return st.st_mtime;
}

#ifdef __cplusplus
}
#endif
