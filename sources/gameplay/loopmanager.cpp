#include "loopmanager.hpp"

#include "../root.hpp"
#include "../global.hpp"
#include "../renderer_list.hpp"
#include "../sdf_renderer.hpp"
#include "../skybox.hpp"
#include "../camera.hpp"
#include "../freecam.hpp"

#include "../visualdebug.hpp"

#include "../opengl_includes.hpp"

#include <SDL.h>
#include <cassert>
#include <algorithm>

namespace Gameplay {

void SmoothTransition::Update(float dt)
{
    const float diff = mTargetValue - mCurrentValue;
    if (fabsf(diff) <= 0.1f)
    {
        SetValue(mTargetValue);
    } else {
        mCurrentValue += dt * diff;
    }
}

float SmoothTransition::GetValue()
{
    return mCurrentValue;
}

void SmoothTransition::SetValue(float value)
{
    mTargetValue = value;
    mCurrentValue = value;
    mTime = 0.f;
}

void SmoothTransition::SetTarget(float value)
{
    mTargetValue = value;
}

LoopManager::LoopManager()
{

}

LoopManager::~LoopManager()
{}

void LoopManager::ListRenderer(std::vector<std::shared_ptr<IRenderer>>& rendererList)
{
    RendererList * renderList = Global::rendererList();
    {
        std::shared_ptr<Skybox> renderer;
        renderer.reset(Skybox::GenerateCheckered());
        renderList->addRenderer(renderer.get());
        rendererList.push_back(renderer);
    }
    {
        std::shared_ptr<SDFRenderer> renderer = std::make_shared<SDFRenderer>();
        renderList->addRenderer(renderer.get());
        rendererList.push_back(renderer);
    }
}

void LoopManager::ListResources(std::vector<Resource *> &resources)
{
}

void LoopManager::OnLoad()
{
}

void LoopManager::Init()
{
    GameSystem* gameSystem = Global::gameSytem();
    Camera* camera = Root::Instance().GetCamera();
    std::unique_ptr<FreeCamera> cameraMover = std::make_unique<FreeCamera>();
    const float an = 12.0 - sin(0.1 * 1.0);
    glm::vec3 ro(3.0 * cos(0.1 * an), 1.0, -3.0 * sin(0.1 * an));
    cameraMover->mPosition = -2.0f * Camera::forward;
    camera->SetCameraMover(std::move(cameraMover));
}

void LoopManager::Terminate()
{

}

void LoopManager::FrameStep()
{
    //VisualDebugSphereCommand command(glm::vec3(0, 0, 0), 0.25f, { 1.f, 1.f, 1.f, 1.f });
    //VisualDebug()->PushCommand(command);
}

void LoopManager::Update(const float deltaTime)
{
}

void LoopManager::OnPhysicsEvent(PhysicEvent & e)
{
}

void LoopManager::Event(const SDL_Event & e)
{

}

void Gameplay::LoopManager::Control(const InputControl& input)
{

}

#if GUI_DEBUG()
void LoopManager::debug_GUI()
{

}
#endif

} //namespace Gameplay
