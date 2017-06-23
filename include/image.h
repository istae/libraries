#pragma once

#include <ctype.h>
#include <assert.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#endif

typedef uint8_t uint8;

#define RGBA 4

struct Image
{
    uint8* pixels = NULL;
    GLuint texture;
    int texture_loaded = 0;
    int width;
    int height;
    int n;
};
