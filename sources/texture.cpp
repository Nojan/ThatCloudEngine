#include "texture.hpp"

#include "image.hpp"
#include "color.hpp"
#include "opengl_includes.hpp"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

constexpr bool is_power_of_two(uint x)
{
    return x && ((x & (x - 1)) == 0);
}

void Texture2D::setTexture(const std::unique_ptr<Color::rgb[]>& data, uint height, uint width)
{
    // TODO avoid copy, or remove this method
    std::unique_ptr<uint8_t[]> d(new uint8_t[3*height*width]);
    for (uint idx = 0; idx < height*width; ++idx)
    {
        d[3*idx+0] = data[idx].r;
        d[3*idx+1] = data[idx].g;
        d[3*idx+2] = data[idx].b;
    }
    mImage.set(d, height, width, ColorsChannel::RGB);
}

uint8_t const * const Texture2D::getData() const
{
    return mImage.data();
}

uint Texture2D::getHeight() const
{
    return mImage.height();
}

uint Texture2D::getWidth() const
{
    return mImage.width();
}

ColorsChannel Texture2D::colorChannel() const
{
    return mImage.channel();
}

GPUBufferHandle & Texture2D::BufferHandle() const
{
    return mBufferHandle;
}

std::unique_ptr<Texture2D> Texture2D::generateUniform(uint height, uint width, Color::rgb color)
{
    assert(is_power_of_two(height));
    assert(is_power_of_two(width));
    const size_t textureSize = height*width;
    std::unique_ptr<Color::rgb[]> data((Color::rgb*)malloc(sizeof(Color::rgb)*textureSize));
    for (size_t x = 0; x < textureSize; ++x)
    {
        data[x] = color;
    }
    std::unique_ptr<Texture2D> texture;
    texture.reset(new Texture2D());
    texture->setTexture(std::move(data), height, width);
    return std::move(texture);
}

std::unique_ptr<Texture2D> Texture2D::generateCheckeredBoard(uint count, uint height, uint width, Color::rgb color1, Color::rgb color2)
{
    assert(is_power_of_two(height));
    assert(is_power_of_two(width));
    const size_t textureSize = height*width;
    const uint checkerHeight = height / count;
    const uint checkerWidth = width / count;
    std::unique_ptr<Color::rgb[]> data((Color::rgb*)malloc(sizeof(Color::rgb)*textureSize));
    for (size_t y = 0; y < height; ++y) 
    {
        const size_t yIndex = y * width;
        const bool yEven = (0 == ((y / checkerHeight) & 1));
        for (size_t x = 0; x < width; ++x)
        {
            const bool xEven = (0 == ((x / checkerWidth) & 1));
            const Color::rgb color = (!yEven && xEven) || (yEven && !xEven) ? color1 : color2;
            data[yIndex + x] = color;
        }
    }
    std::unique_ptr<Texture2D> texture;
    texture.reset(new Texture2D());
    texture->setTexture(std::move(data), height, width);
    return std::move(texture);
}

Texture2D::Texture2D()
{
}

void Texture2D::loadFromFile(const char * imagepath, Texture2D & texture)
{
    Image& image = texture.mImage;
    image.load(imagepath);
    //assert(ColorsChannel::RGB == image.channel());
    //std::unique_ptr<uint8_t[]> data_u8 = std::move(image.data());
    //Color::rgb * color = reinterpret_cast<Color::rgb*>(data_u8.get());
    //std::unique_ptr<Color::rgb[]> data_color(color);
    //data_u8.release();
    // flip x
    //for (size_t y = 0; y < image.height(); ++y)
    //{
    //    const size_t yIndex = y * image.width();
    //    for (size_t x = 0; x < (image.width() / 2); ++x)
    //    {
    //        const size_t xInvert = image.width() - x -1;
    //        std::swap(data_color[yIndex + x], data_color[yIndex + xInvert]);
    //    }
    //}
}

GPUBufferHandle::GPUBufferHandle()
    : mId(-1)
{
}

GPUBufferHandle::~GPUBufferHandle()
{
    FreeResource();
}

bool GPUBufferHandle::valid() const
{
    return -1 != mId;
}

GLuint GPUBufferHandle::Id() const
{
    return mId;
}

void GPUBufferHandle::setId(GLuint id)
{
    //assert(-1 == id || glIsBuffer(id));
    FreeResource();
    mId = id;
}

void GPUBufferHandle::FreeResource()
{
    if (-1 != mId)
    {
        glDeleteBuffers(1, &mId);
        mId = -1;
    }
}
