#include "gamecamera.hpp"

#include "boy.hpp"
#include "../camera.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/ext.hpp>
#include <SDL2/SDL.h>

void BoyCamera::Move(const float speed, Camera* camera)
{
    camera->SetPosition(mBoy->Position() - camera->Direction() * mDistance);
}

void BoyCamera::Event(const SDL_Event& e, Camera* camera)
{
    const bool pressKey = (SDL_KEYDOWN == e.type);
    const bool releaseKey = (SDL_KEYUP == e.type);
    if(pressKey || releaseKey)
    {
        if (SDLK_DOWN == e.key.keysym.sym)
        {
            if (pressKey)
                mMoveMask |= MV_DOWN;
            else
                mMoveMask &= ~MV_DOWN;
        }
        if (SDLK_LEFT == e.key.keysym.sym)
        {
            if (pressKey)
                mMoveMask |= MV_LEFT;
            else
                mMoveMask &= ~MV_LEFT;
        }
        if (SDLK_UP == e.key.keysym.sym)
        {
            if (pressKey)
                mMoveMask |= MV_UP;
            else
                mMoveMask &= ~MV_UP;
        }
        if (SDLK_RIGHT == e.key.keysym.sym)
        {
            if (pressKey)
                mMoveMask |= MV_RIGHT;
            else
                mMoveMask &= ~MV_RIGHT;
        }
    }
    if (SDL_MOUSEBUTTONDOWN == e.type && SDL_BUTTON_RIGHT == e.button.button)
    {
        mMousePan = true;
    }
    else if (SDL_MOUSEBUTTONUP == e.type && SDL_BUTTON_RIGHT == e.button.button)
    {
        mMousePan = false;
    }
    bool positionChanged = false;
    if (SDL_MOUSEWHEEL == e.type)
    {
        const float value(e.wheel.y * 15);
        mDistance = glm::clamp(mDistance - value, 5.f, 500.f);
        positionChanged = true;
    }
    bool orientationChanged = false;
    if (SDL_MOUSEMOTION == e.type)
    {
        const glm::vec2 newMousePosition(static_cast<float>(e.motion.x), static_cast<float>(e.motion.y));
        if(mMousePan)
        {
            const float gain = 0.005f;
            const glm::vec2 vec = (newMousePosition - mMousePosition)*gain;
            mEulerAngle.x -= vec.y;
            mEulerAngle.y += vec.x;
            if(mEulerAngle.x < glm::pi<float>())
                mEulerAngle.x += 2.f * glm::pi<float>();
            if(mEulerAngle.x > glm::pi<float>())
                mEulerAngle.x -= 2.f * glm::pi<float>();
            if(mEulerAngle.y < glm::pi<float>())
                mEulerAngle.y += 2.f * glm::pi<float>();
            if(mEulerAngle.y > glm::pi<float>())
                mEulerAngle.y -= 2.f * glm::pi<float>();

            orientationChanged = true;
        }
        mMousePosition = newMousePosition;
    }

    if(orientationChanged)
    {
        const glm::quat r = glm::normalize(glm::quat(glm::vec3(mEulerAngle, 0.f)));
        camera->SetOrientation(r);
        positionChanged = true;
    }
    if(positionChanged)
    {
        const glm::vec3 position = mOrbitPosition - camera->Direction() * mDistance;
        camera->SetPosition(position);
    }
}
