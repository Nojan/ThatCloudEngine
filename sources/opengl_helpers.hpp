#ifndef OPEN_GL_HELPERS_HPP
#define OPEN_GL_HELPERS_HPP

#include "config.hpp"
#include "opengl_includes.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <type_traits>
#include <vector>

const char* gl_error_enum_string(GLenum);

void gl_log_error();

void glad_setup_callback(bool pre, bool post);

template <GLenum arrayBufferType, GLenum usage, typename T>
void generate_gl_array_buffer(size_t count, GLuint* vboId)
{
    static_assert(GL_ELEMENT_ARRAY_BUFFER == arrayBufferType || GL_ARRAY_BUFFER == arrayBufferType, "arrayBufferType must be GL_ELEMENT_ARRAY_BUFFER or GL_ARRAY_BUFFER");
    static_assert(GL_STREAM_DRAW == usage || GL_DYNAMIC_DRAW == usage || GL_STATIC_DRAW == usage, "usage must be GL_STREAM_DRAW, GL_DYNAMIC_DRAW or GL_STATIC_DRAW");
    const size_t elementSize = sizeof(T);
    glGenBuffers(1, vboId);
    glBindBuffer(arrayBufferType, *vboId);
    glBufferData(arrayBufferType, count * elementSize, 0, usage);
    glBindBuffer(arrayBufferType, 0);
}

template <GLenum arrayBufferType, GLenum usage, typename T>
void update_gl_array_buffer(const T* elements, size_t count, GLuint vboId)
{
    static_assert(GL_ELEMENT_ARRAY_BUFFER == arrayBufferType || GL_ARRAY_BUFFER == arrayBufferType, "arrayBufferType must be GL_ELEMENT_ARRAY_BUFFER or GL_ARRAY_BUFFER");
    static_assert(GL_STREAM_DRAW == usage || GL_DYNAMIC_DRAW == usage || GL_STATIC_DRAW == usage, "usage must be GL_STREAM_DRAW, GL_DYNAMIC_DRAW or GL_STATIC_DRAW");
    const size_t elementSize = sizeof(T);
    glBindBuffer(arrayBufferType, vboId);
    glBufferSubData(arrayBufferType, 0, elementSize * count, elements);
}

template <GLenum arrayBufferType, GLenum usage, typename T>
void update_gl_array_buffer(const std::vector<T>& elements, GLuint vboId)
{
    update_gl_array_buffer<arrayBufferType, usage, T>(elements.data(), elements.size(), vboId);
}

template <GLenum arrayBufferType, GLenum usage, typename T>
void grow_gl_array_buffer(const std::vector<T>& elements, GLuint* vboId, size_t* vboCapacity)
{
    if (*vboCapacity < elements.capacity())
    {
        *vboCapacity = elements.capacity();
        glDeleteBuffers(1, vboId);
        generate_gl_array_buffer<arrayBufferType, usage, T>(*vboCapacity, vboId);
    }
}

#endif
