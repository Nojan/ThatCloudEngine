#pragma once
#include "cameramover.hpp"

#include <glm/glm.hpp>

class FreeCamera : public CameraMover
{
public:
    void Move(const float speed, Camera* camera) override;
    void Event(const SDL_Event& e, Camera* camera) override;

    int mMoveMask = 0;
    bool mMousePan = false;
    glm::vec2 mMousePosition = glm::vec2(0,0);
    glm::vec2 mEulerAngle = glm::vec2(0,0);
};
