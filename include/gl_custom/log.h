#pragma once
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define GL_LOG_FILE "gl.log"

int restart_gl_log() {
  FILE* file = fopen(GL_LOG_FILE, "w");
  if (!file) {
    return 0;
  }

  time_t now = time (NULL);
  char* date = ctime(&now);
  fprintf(file, "GL_LOG_FILE log. local time %s\n", date);
  fclose(file);
  return 1;
}

int gl_log (const char* message, ...) {
  va_list argptr;
  FILE* file = fopen(GL_LOG_FILE, "a");
  if (!file) {
    fprintf(stderr, "ERROR: could not open log file %s", GL_LOG_FILE);
    return 0;
  }

  va_start(argptr, message);
  vfprintf(file, message, argptr);
  va_end(argptr);
  fclose(file);
  return 1;
}

int gl_log_err (const char* message, ...) {
  va_list argptr;
  FILE* file = fopen(GL_LOG_FILE, "a");
  if (!file) {
    fprintf(stderr, "ERROR: could not open log file %s", GL_LOG_FILE);
    return 0;
  }

  va_start (argptr, message);
  vfprintf(file, message, argptr);
  va_end(argptr);

  va_start(argptr, message);
  vfprintf(stderr, message, argptr);
  va_end(argptr);
  fclose(file);
  return 1;
}

void glfw_error_callback (int error, const char* description) {
  gl_log_err("GLFW ERROR: code %i msg: %s\n", error, description);
}

void log_gl_params() {
  GLenum params[] = {
    GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
    GL_MAX_CUBE_MAP_TEXTURE_SIZE,
    GL_MAX_DRAW_BUFFERS,
    GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
    GL_MAX_TEXTURE_IMAGE_UNITS,
    GL_MAX_TEXTURE_SIZE,
    GL_MAX_VARYING_FLOATS,
    GL_MAX_VERTEX_ATTRIBS,
    GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
    GL_MAX_VERTEX_UNIFORM_COMPONENTS,
    GL_MAX_VIEWPORT_DIMS,
    GL_STEREO,
  };
  const char* names[] = {
    "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS",
    "GL_MAX_CUBE_MAP_TEXTURE_SIZE",
    "GL_MAX_DRAW_BUFFERS",
    "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS",
    "GL_MAX_TEXTURE_IMAGE_UNITS",
    "GL_MAX_TEXTURE_SIZE", "GL_MAX_VARYING_FLOATS",
    "GL_MAX_VERTEX_ATTRIBS",
    "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS",
    "GL_MAX_VERTEX_UNIFORM_COMPONENTS",
    "GL_MAX_VIEWPORT_DIMS",
    "GL_STEREO",
  };
  gl_log("GL Context Paramaters:\n");
  for(int i=0; i < 10; ++i) {
    int v=0;
    glGetIntegerv (params[i], &v);
    gl_log("%s %i\n", names[i], v);
  }
  int v[2];
  v[0] = v[1] = 0;
  glGetIntegerv(params[10], v);
  gl_log ("%s %i %i\n", names[ 10], v[ 0], v[ 1]);
  unsigned char s = 0;
  glGetBooleanv (params[ 11], &s);
  gl_log ("%s %u\n", names[ 11], (unsigned int) s);
  gl_log ("-----------------------------\n");
}

void _print_shader_info_log(GLuint shader_index)
{
  int max_len = 2048;
  int actual_len = 0;
  char log[2048];
  glGetShaderInfoLog(shader_index, max_len, &actual_len, log);
  printf("shader info log for GL index %u:\n%s", shader_index, log);
  gl_log("shader info log for GL index %u:\n%s", shader_index, log);
}

void _print_program_info_log(GLuint program_index)
{
  int max_len = 2048;
  int actual_len = 0;
  char log[2048];
  glGetShaderInfoLog(program_index, max_len, &actual_len, log);
  printf("program info log for GL index %u:\n%s", program_index, log);
  gl_log("program info log for GL index %u:\n%s", program_index, log);
}

const char* GL_type_to_string (GLenum type) {
  switch (type) {
    case GL_BOOL: return "bool";
    case GL_INT: return "int";
    case GL_FLOAT: return "float";
    case GL_FLOAT_VEC2: return "vec2";
    case GL_FLOAT_VEC3: return "vec3";
    case GL_FLOAT_VEC4: return "vec4";
    case GL_FLOAT_MAT2: return "mat2";
    case GL_FLOAT_MAT3: return "mat3";
    case GL_FLOAT_MAT4: return "mat4";
    case GL_SAMPLER_2D: return "sampler2D";
    case GL_SAMPLER_3D: return "sampler3D";
    case GL_SAMPLER_CUBE: return "samplerCube";
    case GL_SAMPLER_2D_SHADOW: return "sampler2DShadow";
    default: break;
  }
  return "other";
}

void print_all (GLuint program_index) {
  printf ("--------------------\nshader program_index %i info:\n", program_index);
  int params = -1;
  glGetProgramiv (program_index, GL_LINK_STATUS, &params);
  printf ("GL_LINK_STATUS = %i\n", params);

  glGetProgramiv (program_index, GL_ATTACHED_SHADERS, &params);
  printf ("GL_ATTACHED_SHADERS = %i\n", params);

  glGetProgramiv (program_index, GL_ACTIVE_ATTRIBUTES, &params);
  printf ("GL_ACTIVE_ATTRIBUTES = %i\n", params);
  for (int i = 0; i < params; i++) {
    char name[64];
    int max_length = 64;
    int actual_length = 0;
    int size = 0;
    GLenum type;
    glGetActiveAttrib (
      program_index,
      i,
      max_length,
      &actual_length,
      &size,
      &type,
      name
    );
    if (size > 1) {
      for (int j = 0; j < size; j++) {
        char long_name[64];
        sprintf (long_name, "%s[%i]", name, j);
        int location = glGetAttribLocation (program_index, long_name);
        printf ("  %i) type:%s name:%s location:%i\n",
          i, GL_type_to_string (type), long_name, location);
      }
    } else {
      int location = glGetAttribLocation (program_index, name);
      printf ("  %i) type:%s name:%s location:%i\n",
        i, GL_type_to_string (type), name, location);
    }
  }

  glGetProgramiv (program_index, GL_ACTIVE_UNIFORMS, &params);
  printf ("GL_ACTIVE_UNIFORMS = %i\n", params);
  for (int i = 0; i < params; i++) {
    char name[64];
    int max_length = 64;
    int actual_length = 0;
    int size = 0;
    GLenum type;
    glGetActiveUniform (
      program_index,
      i,
      max_length,
      &actual_length,
      &size,
      &type,
      name
    );
    if (size > 1) {
      for (int j = 0; j < size; j++) {
        char long_name[64];
        sprintf (long_name, "%s[%i]", name, j);
        int location = glGetUniformLocation (program_index, long_name);
        printf ("  %i) type:%s name:%s location:%i\n",
          i, GL_type_to_string (type), long_name, location);
      }
    } else {
      int location = glGetUniformLocation (program_index, name);
      printf ("  %i) type:%s name:%s location:%i\n",
        i, GL_type_to_string (type), name, location);
    }
  }

  _print_program_info_log (program_index);
}

// int is_valid (GLuint program_index) {
//   glValidateProgram (program_index);
//   int params = -1;
//   glGetProgramiv (program_index, GL_VALIDATE_STATUS, &params);
//   printf ("program %i GL_VALIDATE_STATUS = %i\n", program_index, params);
//   if (GL_1 != params) {
//     _print_program_info_log (program_index);
//     return 0;
//   }
//   return 1;
// }
