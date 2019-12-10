#pragma once

#include "classcache.hpp"
#include "texture.hpp"
#include "color.hpp"

class Texture2DCache : public ClassCache<Texture2D>
{
protected:
    std::shared_ptr<Texture2D> load(const std::string& name) const override
    {
        std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>();
        if(name == "default")
            texture = Texture2D::generateCheckeredBoard(8, 128, 128, { 255, 255, 255 }, { 0, 0, 0 });
        else
            Texture2D::loadFromFile(name.c_str(), *texture);
        return texture;
    }
};
