#pragma once

#include "config.hpp"
#include "game_entity.hpp"

#include <atomic>

struct stb_vorbis;

class MusicEntity {
public:
    MusicEntity();
    ~MusicEntity();

    void Init();
    void Terminate();
    void Update(const float deltaTime);

#ifdef IMGUI_ENABLE
    void debug_GUI();
#endif

private:
    GameEntity* mEntity;
    FILE* mFile;
    stb_vorbis* mVorbis;
    float** mVorbisFrame;
    int mVorbisCount;
    int mVorbisIdx;
    float mVolume;
    std::atomic_int mSubmittedFrame;
};
