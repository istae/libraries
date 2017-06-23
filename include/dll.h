#pragma once

#include <Windows.h>
#include <file.h>

void* flib(HMODULE lib, char* func)
{
    return (void*)GetProcAddress(lib, func);
}

HMODULE dll(char* lname, char* tmpname, HMODULE lib)
{
    Sleep(500); // wait for gcc to finish writing file :(
    if (lib)
        FreeLibrary(lib);
    fcpy(tmpname, lname);
    return LoadLibraryA(tmpname);
}
