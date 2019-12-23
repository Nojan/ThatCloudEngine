#pragma once
#include "cameramover.hpp"

#include <glm/glm.hpp>

class FreeCamera : public CameraMover
{
public:
    void Control(const InputControl& control, Camera* camera) override;

    glm::vec3 mPosition = glm::vec3(0, 0, 0);
    glm::vec2 mEulerAngle = glm::vec2(0, 0);
};
