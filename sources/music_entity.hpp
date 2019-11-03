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

#if GUI_DEBUG()
    void debug_GUI();
#endif

private:
    void FreeResource();

    GameEntity* mEntity = nullptr;
    FILE* mFile = nullptr;
    stb_vorbis* mVorbis = nullptr;
    float** mVorbisFrame;
    int mVorbisCount;
    int mVorbisIdx;
    float mVolume;
    std::atomic_int mSubmittedFrame;
};
