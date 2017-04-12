#pragma once
#include <stdarg.h>

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
