#include "camera.hpp"

#include "imgui/imgui_header.hpp"
#include <SDL2/SDL.h>
#include <glm/gtx/transform.hpp>
#include <glm/ext.hpp>

#include <algorithm>
#include <iostream>

#define FREE_CAM
#ifdef __EMSCRIPTEN__
#undef FREE_CAM
#endif

const glm::vec3 Camera::forward = glm::vec3(0.f, 0.f, 1.f);
const glm::vec3 Camera::up = glm::vec3(0, 1, 0);
const glm::vec3 Camera::right = glm::vec3(-1.f, 0, 0);

using namespace std;

#define MV_NONE  0
#define MV_LEFT  1
#define MV_RIGHT 2
#define MV_UP    4
#define MV_DOWN  8

static float deg2rad(const float deg) {
    return deg * 2.f * 3.14159265359f / 360.f;
}

Camera::Camera()
: mUpdateFixedFrameRate(true)
, mUpdateView(true)
, mUpdateProjection(true)
, mMousePan(false)
, mMoveMask(MV_NONE)
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
}

void Camera::Event(const SDL_Event & e)
{
    if(mMover)
        mMover->Event(e, this);

    if (SDL_MOUSEMOTION == e.type)
    {
        const glm::vec2 newMousePosition(static_cast<float>(e.motion.x), static_cast<float>(e.motion.y));
        mMouseDirectionWorld = ProjectScreenCoordToWorld(newMousePosition);
    }
}

void Camera::WindowResize(int width, int height)
{
    mScreenSize = glm::ivec2(max(1, width), max(1, height));
    mPerspective.ratio = static_cast<float>(mScreenSize.x) / static_cast<float>(mScreenSize.y);
    mUpdateProjection = true;
}

#ifdef IMGUI_ENABLE
void Camera::debug_GUI()
{
    ImGui::Text("Position %s", glm::to_string(mPosition).c_str());
    ImGui::Text("Direction %s", glm::to_string(mDirection).c_str());
    ImGui::SliderFloat("Move Speed", &mSpeed, 0.0005f, 0.5f, "%f", 5);

}
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

//const bool orbitCam = false;
//if (orbitCam)
//{
//    glm::vec3 center(-983.503845, 159.502747, -186.739639);
//    const glm::quat r = glm::normalize(glm::quat(glm::vec3(mEulerAngle, 0.f)));
//    mUp = r * up;
//    mOrthoDirection = r * right;
//    mDirection = r * forward;
//    mPosition = center - mDirection * 25.f;
//    mDirection = glm::normalize(center - mPosition);
//}
//else
//{
//    const glm::quat r = glm::normalize(glm::quat(glm::vec3(mEulerAngle, 0.f)));
//    mUp = r * up;
//    mOrthoDirection = r * right;
//    mDirection = r * forward;
//}

void OrbitCamera::Move(const float speed, Camera* camera)
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

void OrbitCamera::Event(const SDL_Event& e, Camera* camera)
{
    if (SDL_MOUSEWHEEL == e.type)
    {
        if (e.wheel.y < 0)
            mDistance += 0.5f;
        else if (e.wheel.y > 0)
            mDistance -= 0.5f;
        mDistance = glm::clamp(mDistance, 5.f, 50.f);
    }
    if (SDL_MOUSEBUTTONDOWN == e.type && SDL_BUTTON_RIGHT == e.button.button)
    {
        mMousePan = true;
    }
    else if (SDL_MOUSEBUTTONUP == e.type && SDL_BUTTON_RIGHT == e.button.button)
    {
        mMousePan = false;
    }
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

            const glm::quat r = glm::normalize(glm::quat(glm::vec3(mEulerAngle, 0.f)));
            camera->SetOrientation(r);
        }
        mMousePosition = newMousePosition;
    }

    {
        glm::vec3 position = camera->Position();
        glm::vec3 center(-983.503845, 159.502747, -186.739639);
        const glm::quat r = glm::normalize(glm::quat(glm::vec3(mEulerAngle, 0.f)));
        camera->SetOrientation(r);
        position = center - camera->Direction() * mDistance;
        camera->SetPosition(position);
    }
}