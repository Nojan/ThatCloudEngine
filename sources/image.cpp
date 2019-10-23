#include "image.hpp"

#include "global.hpp"
#include "platform/platform.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <cassert>

Image::Image()
: mHeight(0)
, mWidth(0)
, mChannel(ColorsChannel::Unknow)
{
}

void Image::load(const char * filepath)
{
    Platform* platform = Global::platform();
    FileHandle fileHandle(platform->OpenFile(filepath, "rb"));
    FILE* file = fileHandle.get();
    assert(file);
    int comp;
    mData.reset(stbi_load_from_file(file, &mWidth, &mHeight, &comp, STBI_default));
    mChannel = static_cast<ColorsChannel>(comp);
}

void Image::set(std::unique_ptr<uint8_t[]>& data, int height, int width, ColorsChannel channel)
{
    assert(0 <= height);
    assert(0 <= width);
    mData = std::move(data);
    mHeight = height;
    mWidth = width;
    mChannel = channel;
}

std::unique_ptr<uint8_t[]> Image::data()
{
    return std::move(mData);
}

uint8_t const * const Image::data() const
{
    return mData.get();
}

int Image::height() const
{
    return mHeight;
}

int Image::width() const
{
    return mWidth;
}

ColorsChannel Image::channel() const
{
    return mChannel;
}
