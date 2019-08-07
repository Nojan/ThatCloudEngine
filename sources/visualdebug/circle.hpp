#pragma once

#include "visualdebug_renderer.hpp"

#include <memory>

class VisualDebugCircleCommand : public IVisualDebugCommand {
public:
    VisualDebugCircleCommand(const glm::vec3& position, const glm::vec3& normal, const float size, const uint subdivision, const Color::rgbap& color);

    void ApplyCommand(std::vector<glm::vec3>& vertexFill, std::vector<Color::rgbap>& colorFill, std::vector<uint>& indexFill,
        std::vector<glm::vec3>& vertexLine, std::vector<Color::rgbap>& colorLine, std::vector<uint>& indexLine) const override;
private:
    glm::vec3 mPosition;
    glm::vec3 mNormal;
    float mSize;
    uint mSubdivision;
    Color::rgbap mColor;
};
