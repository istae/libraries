#pragma once
#include <stdio.h>
#include <string.h>

#define VEC3_MULT(a, b) (a.x * b.x) + (a.y * b.y) + (a.z * b.z)
#define VEC4_SCALE_MULT(a, b) \
    (a.x *= b);               \
    (a.y *= b);               \
    (a.z *= b);               \
    (a.w *= b);

typedef union vec2
{
    struct
    {
        float x, y;
    };
    float e[2];
} vec2;

typedef union vec3
{
    struct
    {
        float x, y, z;
    };
    struct
    {
        float r, g, b;
    };

    vec2 xy;
    float e[3];
} vec3;

typedef union vec4
{
    struct
    {
        float x, y, z, w;
    };
    struct
    {
        float r, g, b, a;
    };
    struct
    {
        vec2 xy, zw;
    };
    vec3 xyz;
    float e[4];
} vec4;

typedef union mat2
{
    struct
    {
        vec2 x, y;
    };
    vec2 col[2];
    float e[4];
} mat2;

typedef union mat3
{
    struct
    {
        vec3 x, y, z;
    };
    vec3 col[3];
    float e[9];
} mat3;

typedef union mat4
{
    struct
    {
        vec4 x, y, z, w;
    };
    vec4 row[4];
    float e[4][4];
} mat4;

mat4 identity()
{
    mat4 m;
    memset(&m, 0, sizeof(mat4));
    m.e[0][0] = 1.0f;
    m.e[1][1] = 1.0f;
    m.e[2][2] = 1.0f;
    m.e[3][3] = 1.0f;
    return m;
}

void translate(mat4 *m, vec3 v)
{
    vec4 cpy = m->w;
    m->e[3][0] += VEC3_MULT(m->x, v);
    m->e[3][1] += VEC3_MULT(m->y, v);
    m->e[3][2] += VEC3_MULT(m->z, v);
    m->e[3][3] += VEC3_MULT(cpy, v);
}

void scale(mat4 *m, vec3 s)
{
    VEC4_SCALE_MULT(m->x, s.x);
    VEC4_SCALE_MULT(m->y, s.y);
    VEC4_SCALE_MULT(m->z, s.z);
}

void debugPrint(mat4 m)
{
    printf("%f, %f, %f, %f\n", m.row[0].x, m.row[0].y, m.row[0].z, m.row[0].w);
    printf("%f, %f, %f, %f\n", m.row[1].x, m.row[1].y, m.row[1].z, m.row[1].w);
    printf("%f, %f, %f, %f\n", m.row[2].x, m.row[2].y, m.row[2].z, m.row[2].w);
    printf("%f, %f, %f, %f\n", m.row[3].x, m.row[3].y, m.row[3].z, m.row[3].w);
}