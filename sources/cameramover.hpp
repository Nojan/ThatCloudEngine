#pragma once

union SDL_Event;
class Camera;

class CameraMover
{
public:
    virtual void Move(const float speed, Camera* camera) {};
    virtual void Event(const SDL_Event& e, Camera* camera) {};

    enum MoveMask
    {
        MV_NONE  = 0,
        MV_LEFT  = 1,
        MV_RIGHT = 2,
        MV_UP    = 4,
        MV_DOWN  = 8,
    };

};
