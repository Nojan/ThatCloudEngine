#include "freecam.hpp"

#include "camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/ext.hpp>
#include <SDL2/SDL.h>

#define FREE_CAM
#ifdef __EMSCRIPTEN__
#undef FREE_CAM
#endif

void FreeCamera::Move(const float speed, Camera* camera)
{
    const glm::vec3 orthoDirection = camera->OrthoDirection();
    const glm::vec3 direction = camera->Direction();
    if (MV_NONE != mMoveMask) 
    {
        glm::vec3 position = camera->Position();
        if (MV_LEFT & mMoveMask)
            position -= orthoDirection*speed;
        if (MV_RIGHT & mMoveMask)
            position += orthoDirection*speed;
        if (MV_UP & mMoveMask)
            position += direction*speed;
        if (MV_DOWN & mMoveMask)
            position -= direction*speed;
        camera->SetPosition(position);
    }
}

void FreeCamera::Event(const SDL_Event& e, Camera* camera)
{
#ifdef FREE_CAM
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
    if (SDL_MOUSEWHEEL == e.type)
    {
        Camera::perspective p = camera->Perspective();
        if (e.wheel.y < 0)
            p.fov += 0.1f;
        else if (e.wheel.y > 0)
            p.fov -= 0.1f;
        camera->SetPerspective(p);
    }
#endif
    if (SDL_MOUSEMOTION == e.type)
    {
        const glm::vec2 newMousePosition(static_cast<float>(e.motion.x), static_cast<float>(e.motion.y));
        if (mMousePan)
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

            const glm::quat r = glm::normalize(glm::quat(glm::vec3(mEulerAngle, 0.f)));
            camera->SetOrientation(r);
        }
        mMousePosition = newMousePosition;
    }
}
