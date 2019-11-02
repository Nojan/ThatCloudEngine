#pragma once

#include "opengl_helpers.hpp"

#include <vector>

template <GLenum arrayBufferType, typename T>
class VBO_dynamic {
public:
    ~VBO_dynamic();

    void StreamGPU();

    std::vector<T> mElements;
    GLuint mVboId = 0;
    size_t mVboCapacity = 0;
};

template <GLenum arrayBufferType, typename T>
VBO_dynamic<arrayBufferType, T>::~VBO_dynamic()
{
    glDeleteBuffers(1, &mVboId);
}

template <GLenum arrayBufferType, typename T>
void VBO_dynamic<arrayBufferType, T>::StreamGPU()
{
    const size_t size = mElements.size();
    if(size < 0)
        return;

    grow_gl_array_buffer<arrayBufferType, GL_DYNAMIC_DRAW, T>(mElements, &mVboId, &mVboCapacity);
    update_gl_array_buffer<arrayBufferType, GL_DYNAMIC_DRAW, T>(mElements, mVboId);
}
