#pragma once

#include <glm/glm.hpp>

union SDL_Event;
class Camera;
struct InputControl;

class CameraMover
{
public:
    virtual void Move(const float speed, Camera* camera) {};
    virtual void Control(const InputControl& control, Camera* camera) {};
    virtual void Event(const SDL_Event& e, Camera* camera) {};

    virtual void GetTransform(glm::vec3& position, glm::quat& orientation) = 0;

    enum MoveMask
    {
        MV_NONE  = 0,
        MV_LEFT  = 1,
        MV_RIGHT = 2,
        MV_UP    = 4,
        MV_DOWN  = 8,
    };

};
