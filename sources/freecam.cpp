#include "freecam.hpp"

#include "camera.hpp"
#include "input_control.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/ext.hpp>

void FreeCamera::Control(const InputControl& control, Camera* camera)
{
    bool positionChanged = false;
    if (0.f != glm::dot(control.move, control.move))
    {
        const glm::vec3& direction = camera->Direction();
        const glm::vec3& orthoDirection = camera->OrthoDirection();
        mPosition += (direction * -control.move.y) + (orthoDirection * control.move.x);
        positionChanged = true;
    }
    bool orientationChanged = false;
    if (0.f != glm::dot(control.view, control.view))
    {
        mEulerAngle.x += control.view.y;
        mEulerAngle.y -= control.view.x;
        if (mEulerAngle.x < glm::pi<float>())
            mEulerAngle.x += 2.f * glm::pi<float>();
        if (mEulerAngle.x > glm::pi<float>())
            mEulerAngle.x -= 2.f * glm::pi<float>();
        if (mEulerAngle.y < glm::pi<float>())
            mEulerAngle.y += 2.f * glm::pi<float>();
        if (mEulerAngle.y > glm::pi<float>())
            mEulerAngle.y -= 2.f * glm::pi<float>();

        orientationChanged = true;
    }
    if (orientationChanged)
    {
        const glm::quat r = glm::normalize(glm::quat(glm::vec3(mEulerAngle, 0.f)));
        camera->SetOrientation(r);
        positionChanged = true;
    }
    if (positionChanged)
    {
        camera->SetPosition(mPosition);
    }
}

void FreeCamera::GetTransform(glm::vec3& position, glm::quat& orientation)
{
    orientation = glm::normalize(glm::quat(glm::vec3(mEulerAngle, 0.f)));
    position = mPosition;
}
