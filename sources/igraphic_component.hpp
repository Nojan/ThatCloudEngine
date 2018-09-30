#pragma once

#include "color.hpp"

class TransformComponent;

template<typename TRenderer>
class IGraphicComponent
{
public:
    TransformComponent* mTransformComponent = nullptr;
    Color::rgbap mColor;
    bool mEnable = true;
};
