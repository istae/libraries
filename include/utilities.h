 #pragma once

// function parameters are evaluated from right to left, rofl who knew

// needs
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define ERROR_EXIT(str) do {fprintf(stderr, str); exit(1);} while(0)
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;

void error_exit(char* str)
{
    fprintf(stderr, str);
    exit(1);
}

void debug(char* str, ...)
{
    #ifdef DEBUG
        #if DEBUG
        va_list args;
        va_start(args, str);
        vprintf(str, args);
        va_end(args);
        #endif
    #endif
}

int b64toa(char* str, char* b64, int len)
{
    char base64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int value[128];
    for(int i=0; i < sizeof(base64); i++)
        value[base64[i]] = i;

    int j = 0;
    int step = 0;

    for(int i=0; i < len-1; step++) {

        if (step == 0)
            str[j++] = (value[b64[i++]] << 2) | (value[b64[i]] >> 4);

        else if (step == 1)
            str[j++] = (value[b64[i++]] << 4) | (value[b64[i]] >> 2);

        else {
            str[j++] = (value[b64[i++]] << 6) | (value[b64[i]]);
            step = -1;
            i++;
        }
    }

    str[j] = '\0';
    return j;
}

int atob64(char* b64, char* str, int len)
{
    char base64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    uint8 TWO_BITS  = 0x03; // 0000 0011
    uint8 FOUR_BITS = 0x0F; // 0000 1111
    uint8 SIX_BITS  = 0x3F; // 0011 1111

    int j = 0;
    int carry = 0;
    int step = 0;

    for(int i=0; i < len+1; i++, step++) {

        uint8 c = str[i];
        if (step == 0) {
            b64[j++] = base64[c >> 2];
            carry = c & TWO_BITS;
        }

        else if (step == 1) {
            b64[j++] = base64[ (c >> 4) | (carry << 4) ];
            carry = c & FOUR_BITS;
        }

        else if (step == 2) {
            b64[j++] = base64[ (c >> 6) | (carry << 2) ];
            carry = c & SIX_BITS;
        }

        else {
            b64[j++] = base64[carry];
            step = -1;
            --i;
        }
    }

    if (carry)
        b64[j++] = base64[carry];

    b64[j] = '\0';
    return j;
}

// reads input from stdin
int _getline(FILE* f, char* b, int max)
{
    int i, c;
    for (i = 0; i < max && (c=getc(f)) != EOF && c != '\n'; i++)
        b[i] = c;
    if (i == 0 && c == '\n')
        b[i++] = '\n';
    b[i] = '\0';
    return i;
}

int fset(char* path, void* buffer, int len)
{
    FILE* f = fopen (path, "wb");
    if (!f) return 0;
    fwrite (buffer , 1, len, f);
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
        buffer = malloc(length+1); // + '\0'
        if (buffer != NULL) {
            fread (buffer, 1, length, f);
            buffer[length] = '\0';
            *len = length;
        }
        fclose (f);
    }
    return buffer;
}

// not lazy realloc
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

//  MODIFIES STR!
// split words, ends at eof or \n
// must free return value
/*
    2 WAYS TO PRINT

    char str[] = " aa       sass ";
    char** lines = split(str, &len);

1)
    for(int i=0; i < len; i++)
        printf("%s", lines[i]);

2)
    char* p = str;
    for (int i=0; i < len; i++) {
        printf("%s", p);
        p += strlen(p) + 1;
    }

    free(lines);
*/
char** split(char* str, int* count)
{
    int j=0;
    int c=0;

    int size = 1;
    char** lines = malloc(sizeof(char*));

    for(int i=0; str[i] != '\0'; i++) {

        int prev = j;

        while (str[i] != '\0' && (!isspace(str[i])))
            str[j++] = str[i++];

        if (prev != j) {
            str[j++] = '\0';
            lines[c++] = str+prev;

            // realloc
            if (c == size) {
                size *= 2;
                if (!safe_realloc((void*)&lines, sizeof(char*) * size)) { // (void*) because gcc keeps crying about it
                    return NULL;
                }
            }
        }
    }
    *count = c;
    return lines;
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

void _p_swap(void* a, void* b, const size_t size)
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
            _p_swap(a+(i*size), a+(len*size), size);
        }
    }
}

// little endian memmory cmp
// returns 0 of equal, 1 if a > b. -1 if a < b
// extern inline
int litend_memcmp(void* a, void*b, int s)
{
    while(--s >= 0) {
        uint8* x = a + s;
        uint8* y = b + s;
        if (*x > *y)
            return 1;
        else if (*x < *y)
            return -1;
    }
    return 0;
}

// 1) pointer 2) array length 3) item to find 4) item size
int lower_bound(void* a, const int len, void* m, const int size)
{
    int b = 0;
    int e = len-1;
    int mid = (e + b) / 2;
    while (b <= e) {
        int cmp = litend_memcmp(a + (mid * size), m, size);
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

int lower_bound_int(int* n, const int len, int m)
{
    int b = 0;
    int e = len-1;
    int mid = (e + b) / 2;
    while (b <= e) {
        if (n[mid] < m)
            b = mid+1;
        else if (n[mid] > 0)
            e = mid - 1;
        else
            return mid;
        mid = (e + b) / 2;
    }
    return b;
}

int binary_search_int(int* a, const int len, int m)
{
    int pos = lower_bound_int(a, len, m);
    if (pos < len && a[pos] == m)
        return 1;
    return 0;
}

int binary_search(void* a, int len, void* m, int size)
{
    int pos = lower_bound(a, len, m, size);
    if (pos < len && !litend_memcmp(a + (pos * size), m, size))
        return 1;
    else
        return 0;
}

int find(void* a, int len, void* m, int size)
{
    for (int i=0; i < len; i++) {
        if (memcmp(a+i*size, m, size)==0)
            return i;
    }
    return -1;
}

// give this an array of int, and see how bits are organized in memory
int mem_print(void* a, int len, int size)
{
    for (int i=1; i < len; i++) {
        uint8* x = a + ((i - 1) * size);
        uint8* y = a + (i * size);
        printf("%d %d\n", *(int*)(a + ((i - 1) * size)), *(int*)(a + (i * size)));
        printf("----------------\n");
        for (int j=size-1; j >= 0; j--) {
            printf("%p: %d\t%p: %d\n", x+j, x[j], y+j, y[j]);
        }
        printf("\n");
    }
    return 1;
}

// little endian vs big endian issue (first time i came across it :)
// turns out my machine is little ;)
int issorted(void* a, int len, int size)
{
    for (int i=1; i < len; i++)
        if (litend_memcmp(a + ((i-1) * size), a + (i * size), size) == 1)
            return 0;
    return 1;
}

int issorted_int(int* a, int len)
{
    for (int i=1; i < len; i++)
        if (a[i] < a[i-1])
            return 0;
    return 1;
}

void printBits(size_t size, int skip, void* ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    for (int i=size-1; i>=0; i--)
    {
        int j;
        for(j=7; j>=skip; j--)
        if ((b[i] >> j) & 1)
            break;

        for (int k=j; k>=0; k--)
        {
            byte = (b[i] >> k) & 1;
            printf("%u", byte);
        }
        printf(" ");
    }
}
