#ifndef OPENGL_INCLUDE_H
#define OPENGL_INCLUDE_H

#define OPENGL_ES2

#ifndef __EMSCRIPTEN__
#define USE_GLAD
#endif

#ifdef USE_GLAD
#include "glad/glad.h"
#endif
#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#endif
#include "opengl_helpers.hpp"

#endif