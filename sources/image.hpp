#pragma once
#include "types.hpp"

#include <memory>

enum class ColorsChannel {
    Unknow,
    Grey,
    GreyAlpha,
    RGB,
    RGBA
};

class Image
{
public:
    Image();

    void load(const char * filepath);
    void set(std::unique_ptr<uint8_t[]>& data, int height, int width, ColorsChannel channel);

    std::unique_ptr<uint8_t[]> data();

    uint8_t const * const data() const;
    int height() const;
    int width() const;
    ColorsChannel channel() const;
private:
    std::unique_ptr<uint8_t[]> mData;
    int mHeight;
    int mWidth;
    ColorsChannel mChannel;
};
