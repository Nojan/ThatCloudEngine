#pragma once
#include "cameramover.hpp"

#include <glm/glm.hpp>

class OrbitCamera : public CameraMover
{
public:
    void Move(const float speed, Camera* camera) override;
    void Event(const SDL_Event& e, Camera* camera) override;

    void GetTransform(glm::vec3& position, glm::quat& orientation) override;

    int mMoveMask = 0;
    bool mMousePan = false;
    glm::vec3 mOrbitPosition = glm::vec3(-983.503845, 159.502747, -186.739639);
    glm::vec2 mMousePosition = glm::vec2(0,0);
    glm::vec2 mEulerAngle = glm::vec2(0,0);
    float mDistance = 5.f;
};
