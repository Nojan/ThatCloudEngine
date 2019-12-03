#include "gamecamera.hpp"

#include "boy.hpp"
#include "../camera.hpp"
#include "../input_control.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/ext.hpp>
#include <SDL.h>

void BoyCamera::Move(const float speed, Camera* camera)
{
    camera->SetPosition(mBoy->Position() - camera->Direction() * mDistance);
}

void BoyCamera::Control(const InputControl & control, Camera* camera)
{
    bool positionChanged = false;
    if (0 != control.zoom)
    {
        mDistance = glm::clamp(mDistance - control.zoom, 5.f, 500.f);
        positionChanged = true;
    }
    bool orientationChanged = false;
    if (0.f != glm::dot(control.view, control.view))
    {
        mEulerAngle.x -= control.view.y;
        mEulerAngle.y += control.view.x;
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

