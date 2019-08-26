#pragma once

#include "../iupdater.hpp"
#include "../physics_event.hpp"
#include "../imgui/imgui_header.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

union SDL_Event;

class Texture2D;
class MusicEntity;
class GameEntity;
class Boy;
class Cursor;

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

class LoopManager : public IUpdater, PhysicsListener {
public:
    LoopManager();
    ~LoopManager();

    void Init();
    void Terminate();
    void FrameStep() override;
    void Update(const float deltaTime) override;
    void OnPhysicsEvent(PhysicEvent& e) override;

    void SpawnCloud(const glm::vec3& position, const int color, const float power);

    void Event(const SDL_Event& e);
    void OnMotion(const float x, const float y);
    glm::vec2 Motion() const { return mMotion; }

    enum soundEffectIdx {
        CloudRelease,
        CloudConsume,
        CloudNormalPurified,
    };
    void PlaySoundEffect(soundEffectIdx idx);

#ifdef IMGUI_ENABLE
    void debug_GUI();
#endif

private:
    std::unique_ptr<MusicEntity> mMusic;
    std::unique_ptr<GameEntity> mSoundEffects;
    std::unique_ptr<Boy> mBoy;
    std::unique_ptr<Cursor> mCursor;
    std::vector< GameEntity* > mEntities;
    std::vector<std::shared_ptr<Texture2D>> mCloudsTextures;
    SmoothTransition mAdditionalRadius;
    glm::vec2 mMotion = glm::vec2(0,0);
    int mCloudTextureIdx = 0;
    int mGridCount = 0;
    int mGridFilled = 0;
    int mGridUpdateIdx = 0;
    int mCloudCount = 0;
    float mStoredCloud = 0.f;
    GameDebugMode mGameDebugMode = GameDebugMode::None;
    bool mClickLeft = false;
    bool mShiftLeft = false;
    bool mCtrlLeft = false;
};

} // namespace
