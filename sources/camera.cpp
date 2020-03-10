#include "camera.hpp"

#include "cameramover.hpp"
#include "freecam.hpp"
#include "orbitcamera.hpp"

#include "imgui/imgui_header.hpp"
#include <SDL.h>
#include <glm/gtx/transform.hpp>
#include <glm/ext.hpp>

#include <algorithm>
#include <iostream>

const glm::vec3 Camera::forward = glm::vec3(0.f, 0.f, 1.f);
const glm::vec3 Camera::up = glm::vec3(0, 1, 0);
const glm::vec3 Camera::right = glm::vec3(-1.f, 0, 0);

using namespace std;

static float deg2rad(const float deg) {
    return deg * 2.f * 3.14159265359f / 360.f;
}

Camera::Camera()
: mUpdateFixedFrameRate(true)
, mUpdateView(true)
, mUpdateProjection(true)
, mSpeed(0.1f)
, mScreenSize(1, 1)
, mMouseDirectionWorld(0.f)
, mPosition(-668.f, 44.f, 833.f)
, mDirection(glm::normalize(glm::vec3(0.7f, -0.1f, 0.6f)))
, mUp(0.f, 1.f, 0.f)
, mOrthoDirection(glm::cross(mDirection, mUp))
{
    OrbitCamera* freecam = new OrbitCamera();
    freecam->mEulerAngle = glm::vec2(glm::eulerAngles(glm::quat(forward, mDirection)));
    SetOrientation(glm::normalize(glm::quat(glm::vec3(freecam->mEulerAngle, 0.f))));
    mMover.reset(freecam);
    
    mPerspective.fov = deg2rad(45.f);
    mPerspective.ratio = 4.f/3.f;
    mPerspective.zNear = 1.f;
    mPerspective.zFar = 2000.f;

    Update(1.f);
}

Camera::~Camera()
{}

void Camera::FrameStep()
{
    if (mUpdateFixedFrameRate)
    {
        const float frameDuration = 16.f / 1000.f;
        Move(mSpeed / frameDuration);
    }
}

void Camera::Update(const float frameDuration)
{
    if (!mUpdateFixedFrameRate)
    {
        Move(mSpeed / frameDuration);
    }
}

void Camera::Move(const float speed)
{
    if(mMover)
        mMover->Move(speed, this);

    if (mUpdateView) {
        glm::vec3 center = mPosition + mDirection;
        mView = glm::lookAt(mPosition, center, mUp);
        mViewInv = glm::inverse(mView);
    }
    if (mUpdateProjection) {
        mProjection = glm::perspective(mPerspective.fov, mPerspective.ratio, mPerspective.zNear, mPerspective.zFar);
        mProjectionInv = glm::inverse(mProjection);
    }
    if (mUpdateView || mUpdateProjection) {
        mProjectionView = mProjection*mView;
        mProjectionViewInv = glm::inverse(mProjectionView);
        mUpdateView = false;
        mUpdateProjection = false;
    }
}

Camera::frustum Camera::ConvertTo(Camera::perspective const & perspective)
{
    Camera::frustum f;
    f.zNear = perspective.zNear;
    f.zFar = perspective.zFar;
    const float tanHalfFovy = glm::tan(perspective.fov / 2.f);
    f.top = tanHalfFovy;
    f.bottom = -f.top;
    f.right = perspective.ratio * tanHalfFovy;
    f.left = -f.right;
    return f;
}

Camera::perspective const& Camera::Perspective() const
{
    return mPerspective;
}

void Camera::SetPerspective(Camera::perspective const& p)
{
    mPerspective = p;
    mUpdateProjection = true;
}

glm::ivec2 const& Camera::ScreenSize() const
{
    return mScreenSize;
}

glm::vec3 const& Camera::Position() const
{
    return mPosition;
}

void Camera::SetPosition(glm::vec3 const& position)
{
    mPosition = position;
    mUpdateView = true;
}

void Camera::SetOrientation(const glm::quat& orientation)
{
    mUp = orientation * up;
    mOrthoDirection = orientation * right;
    mDirection = orientation * forward;
    mUpdateView = true;
}

void Camera::GetTransform(glm::vec3& position, glm::quat& orientation) const
{
    mMover->GetTransform(position, orientation);
}

glm::vec3 const& Camera::OrthoDirection() const
{
    return mOrthoDirection;
}

glm::vec3 const& Camera::Direction() const
{
    return mDirection;
}

glm::vec3 const& Camera::MouseDirection() const
{
    return mMouseDirectionWorld;
}

glm::vec3 Camera::ProjectScreenCoordToWorld(const glm::vec2 & screenCoord) const
{
    const float x = (2.f * screenCoord.x) / mScreenSize.x - 1.f;
    const float y = 1.f - (2.f * screenCoord.y) / mScreenSize.y;
    const glm::vec2 screenCoordNormalized(x, y);  
    return ProjectScreenCoordNormalizedToWorld(screenCoordNormalized);
}

glm::vec3 Camera::ProjectScreenCoordNormalizedToWorld(const glm::vec2 & screenCoord) const
{
    const glm::vec3 ray_nds(screenCoord.x, screenCoord.y, 1.f);
    const glm::vec4 ray_clip(ray_nds.x, ray_nds.y, -1.f, 1.f);
    const glm::vec4 ray_proj = mProjectionInv * ray_clip;
    const glm::vec4 ray_eye(ray_proj.x, ray_proj.y, -1.f, 0.f);
    const glm::vec4 ray_wor = mViewInv * ray_eye;
    const glm::vec3 direction(ray_wor.x, ray_wor.y, ray_wor.z);
    return glm::normalize(direction);
}

glm::vec3 const& Camera::Up() const
{
    return mUp;
}

glm::mat4 const& Camera::View() const
{
    return mView;
}

glm::mat4 const& Camera::ViewInv() const
{
    return mViewInv;
}

glm::mat4 const& Camera::Projection() const
{
    return mProjection;
}

glm::mat4 const& Camera::ProjectionView() const
{
    return mProjectionView;
}

glm::mat4 const & Camera::ProjectionViewInv() const
{
    return mProjectionViewInv;
}

void Camera::SetCameraMover(std::unique_ptr<CameraMover>&& mover)
{
    mMover = std::move(mover);
    if (mMover)
    {
        glm::vec3 position;
        glm::quat orientation;
        mMover->GetTransform(position, orientation);
        SetOrientation(orientation);
        SetPosition(position);
    }
}

void Camera::Event(const SDL_Event & e)
{
    if (SDL_MOUSEMOTION == e.type)
    {
        const glm::vec2 newMousePosition(static_cast<float>(e.motion.x), static_cast<float>(e.motion.y));
        mMouseDirectionWorld = ProjectScreenCoordToWorld(newMousePosition);
    }
}

void Camera::Control(const InputControl& control)
{
    if(mMover)
        mMover->Control(control, this);
}

void Camera::WindowResize(int width, int height)
{
    mScreenSize = glm::ivec2(max(1, width), max(1, height));
    mPerspective.ratio = static_cast<float>(mScreenSize.x) / static_cast<float>(mScreenSize.y);
    mUpdateProjection = true;
}

#if GUI_DEBUG()
void Camera::debug_GUI()
{
    ImGui::Text("Position %s", glm::to_string(mPosition).c_str());
    ImGui::Text("Direction %s", glm::to_string(mDirection).c_str());
    ImGui::SliderFloat("Move Speed", &mSpeed, 0.0005f, 0.5f, "%f", 5);

}
#endif
