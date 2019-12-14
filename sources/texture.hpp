#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "types.hpp"
#include "image.hpp"
#include "opengl_helpers.hpp"

#include <memory>

class GPUBufferHandle {
public:
    GPUBufferHandle();
    ~GPUBufferHandle();

    bool valid() const;

    GLuint Id() const;
    void setId(GLuint id);

private:
    void FreeResource();
    GLuint mId;
};

namespace Color
{
    struct rgb;
    struct rgba;
}

class Texture2D
{
public:
    Texture2D();
    ~Texture2D() = default;

    static void loadFromFile(const char * imagepath, Texture2D & texture);
    static std::unique_ptr<Texture2D> generateUniform(uint height, uint width, Color::rgb color);
    static std::unique_ptr<Texture2D> generateCheckeredBoard(uint count, uint height, uint width, Color::rgb color1, Color::rgb color2);

    void setTexture(const Color::rgb* data, uint height, uint width);

    uint8_t const * const getData() const;
    uint getHeight() const;
    uint getWidth() const;
    ColorsChannel colorChannel() const;

    GPUBufferHandle& BufferHandle() const;

private:
    Image mImage;
    mutable GPUBufferHandle mBufferHandle;
};

#endif
