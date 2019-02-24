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
    int mCloudTextureIdx = 0;
    int mGridCount = 0;
    int mGridFilled = 0;
    int mGridUpdateIdx = 0;
    float mStoredCloud = 0.f;
    bool mClickLeft = false;
    bool mShiftLeft = false;
    bool mCtrlLeft = false;
};

} // namespace
