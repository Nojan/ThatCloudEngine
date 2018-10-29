#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "config.hpp"
#include "iupdater.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

union SDL_Event;
class CameraMover;

class Camera : public IUpdater {
public:
    struct perspective{
        float fov;
        float ratio;
        float zNear;
        float zFar;
    };

    struct frustum {
        float left;
        float right;
        float bottom;
        float top;
        float zNear;
        float zFar;
    };

    static const glm::vec3 forward;
    static const glm::vec3 up;
    static const glm::vec3 right;

    Camera();
    ~Camera();

    void FrameStep() override;
    void Update(const float frameDuration) override;
    void Move(const float speed);

    static frustum ConvertTo(perspective const& perspective); 
    perspective const& Perspective() const;
    void SetPerspective(perspective const& p);
    glm::ivec2 const& ScreenSize() const;

    glm::vec3 const& Position() const;
    void SetPosition(glm::vec3 const& position);

    void SetOrientation(const glm::quat& orientation);

    glm::vec3 const& OrthoDirection() const;
    glm::vec3 const& Direction() const;
    glm::vec3 const& Up() const;

    glm::vec3 const& MouseDirection() const;
    glm::vec3 ProjectScreenCoordToWorld(const glm::vec2& screenCoord) const;
    glm::vec3 ProjectScreenCoordNormalizedToWorld(const glm::vec2& screenCoord) const;

    glm::mat4 const& View() const;
    glm::mat4 const& ViewInv() const;
    glm::mat4 const& Projection() const;
    glm::mat4 const& ProjectionView() const;
    glm::mat4 const& ProjectionViewInv() const;

    void SetCameraMover(std::unique_ptr<CameraMover>&& mover);

    void Event(const SDL_Event& e);
    
    void WindowResize(int width, int height);

#ifdef IMGUI_ENABLE
    void debug_GUI();
#endif

private:
    bool mUpdateFixedFrameRate;
    bool mUpdateView;
    bool mUpdateProjection;
    bool mMousePan;

    int mMoveMask;
    float mSpeed;

    glm::ivec2 mScreenSize;

    perspective mPerspective;

    glm::vec3 mMouseDirectionWorld;

    glm::vec3 mPosition;
    glm::vec3 mDirection;
    glm::vec3 mOrthoDirection;
    glm::vec3 mUp;

    glm::mat4 mView;
    glm::mat4 mProjection;
    glm::mat4 mProjectionView;
    glm::mat4 mViewInv;
    glm::mat4 mProjectionInv;
    glm::mat4 mProjectionViewInv;

    std::unique_ptr<CameraMover> mMover;
};


class CameraMover
{
public:
    virtual void Move(const float speed, Camera* camera) {};
    virtual void Event(const SDL_Event& e, Camera* camera) {};
};

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

class OrbitCamera : public CameraMover
{
public:
    void Move(const float speed, Camera* camera) override;
    void Event(const SDL_Event& e, Camera* camera) override;

    int mMoveMask = 0;
    bool mMousePan = false;
    glm::vec2 mMousePosition = glm::vec2(0,0);
    glm::vec2 mEulerAngle = glm::vec2(0,0);
    float mDistance = 5.f;
};

#endif
