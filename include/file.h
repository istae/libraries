#pragma once
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
/*
C ONLY
*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>

int isdir(const char* path)
{
    // struct stat st;
    // stat(fname, &st); // error check laer
    // return ((st.st_mode & S_IFMT) == S_IFDIR);
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

enum {
    FILES_ONLY,
    DIRS_ONLY,
    FILES_AND_DIRS,
    MAX_FILE_PATHS = 256,
    MAX_FILE_NAME = 256,
};

//expects an 2d array with MAX_FILE_NAME length rows
int dirlist(const char* dir, int flg, char dlist[][MAX_FILE_PATHS])
{
    // MAX_PATH  = 255
    DIR* dfd = opendir(dir);
    if (dfd == NULL) {
        fprintf(stderr, "cannot access %s\n", dir);
        exit(1);
    }

    struct dirent* dp;
    int i = 0;

    while ((dp = readdir(dfd)) != NULL ) {
        if (strcmp(dp->d_name, ".") == 0)
            continue;
        if (flg == FILES_AND_DIRS)
            strcpy(dlist[i++], dp->d_name);
        else {

            char path[1000];
            sprintf(path, "%s/%s", dir, dp->d_name);

            int is = isdir(path);

            if (flg == FILES_ONLY && (!is))
                strcpy(dlist[i++], dp->d_name);

                else if (flg == DIRS_ONLY && is)
                strcpy(dlist[i++], dp->d_name);
        }
    }
    closedir(dfd);
    return i;
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

int fcpy(char* f1, char* f2)
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
