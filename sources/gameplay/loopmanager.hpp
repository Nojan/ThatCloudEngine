#pragma once

#include "../iupdater.hpp"
#include "../iresourceowner.hpp"
#include "../physics_event.hpp"
#include "../imgui/imgui_header.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

union SDL_Event;

class Texture2D;
class IRenderer;

struct InputControl;

namespace Gameplay {

class SmoothTransition {
public:
    void Update(float dt);
    float GetValue();
    void SetValue(float value);
    void SetTarget(float value);
    
private:
    float mTargetValue = 0.f;
    float mCurrentValue = 0.f;
    float mTime = 0.f;
};

enum GameDebugMode : uint8_t {
    None,
    Dot,
    Volume,
};

class LoopManager : public IUpdater, PhysicsListener, IResourceOwner {
public:
    LoopManager();
    ~LoopManager();

    void ListRenderer(std::vector<std::shared_ptr< IRenderer > >& rendererList);

    void ListResources(std::vector<Resource*>& resources) override;
    void OnLoad() override;

    void Init();
    void Terminate();
    void FrameStep() override;
    void Update(const float deltaTime) override;
    void OnPhysicsEvent(PhysicEvent& e) override;

    void Event(const SDL_Event& e);
    void Control(const InputControl& input);

#if GUI_DEBUG()
    void debug_GUI();
#endif

private:
};

} // namespace
