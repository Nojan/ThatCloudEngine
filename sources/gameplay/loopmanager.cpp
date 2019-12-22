#include "loopmanager.hpp"

#include "../root.hpp"
#include "../global.hpp"
#include "../renderer_list.hpp"

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
}

void LoopManager::Terminate()
{

}

void LoopManager::FrameStep()
{}

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
