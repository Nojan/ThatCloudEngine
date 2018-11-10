#pragma once

#include <glm/glm.hpp>
#include <memory>

class Texture2D;

class Billboard {
public:

    ~Billboard() = default;

    glm::vec3 mPosition;
    glm::vec2 mSize;
    float     mAlpha;
    std::shared_ptr< Texture2D > mTexture;
};
