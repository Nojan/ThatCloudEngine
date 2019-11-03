#include "music_entity.hpp"

#include "global.hpp"
#include "game_system.hpp"
#include "sound_system.hpp"
#include "platform/platform.hpp"
#include "vorbis.h"
#include "imgui/imgui_header.hpp"

#include <cassert>

MusicEntity::MusicEntity()
: mSubmittedFrame(0)
, mVolume(1.f)
{}

MusicEntity::~MusicEntity()
{
    assert(nullptr == mEntity);
    assert(nullptr == mVorbis);
}

void MusicEntity::Init()
{
    GameSystem* gameSystem = Global::gameSytem();
    mEntity = gameSystem->createEntity();
    gameSystem->getSystem<SoundSystem>()->attachEntity(mEntity);
    mFile = Global::platform()->OpenFile("../assets/Sounds/ingame.ogg", "rb");
    assert(mFile);
    mVorbis = stb_vorbis_open_file(mFile, false, nullptr, nullptr);
    assert(mVorbis);
    mVorbisCount = 0;
    mVorbisIdx = 0;
    mSubmittedFrame = 0;

    const stb_vorbis_info vorbis_info = stb_vorbis_get_info(mVorbis);
    assert(vorbis_info.channels == 1 || vorbis_info.channels == 2); // either mono or stereo
}

void MusicEntity::Terminate()
{
    GameSystem* gameSystem = Global::gameSytem();
    gameSystem->removeEntity(mEntity);
    mEntity = nullptr;
    FreeResource();
}

void MusicEntity::Update(const float deltaTime)
{
    if(nullptr == mVorbis)
        return;
    const stb_vorbis_info vorbis_info = stb_vorbis_get_info(mVorbis);
    const uint8_t channelCount = vorbis_info.channels;
    GameSystem* gameSystem = Global::gameSytem();
    SoundSystem* soundSystem = gameSystem->getSystem<SoundSystem>();
    int musicDone = 0;
    assert(0 <= mSubmittedFrame);
    while (musicDone < 2 && mSubmittedFrame < (2 * SoundFrame::sample_size))
    {
        bool hasEnoughChannel = true;
        std::vector<SoundFrame*> sFramesPerChannel;
        sFramesPerChannel.reserve(channelCount);
        for (uint8_t channel = 0; channel < channelCount && hasEnoughChannel; ++channel)
        {
            SoundFrame* soundFrame = soundSystem->RequestFrame();
            hasEnoughChannel = nullptr != soundFrame;
            soundFrame->mPan = 0 == channel ? -1.f : 1.f;
            sFramesPerChannel.push_back(soundFrame);
        }
        if (!hasEnoughChannel)
        {
            for (size_t channel = 0; channel < sFramesPerChannel.size(); ++channel)
            {
                if (SoundFrame* soundFrame = sFramesPerChannel[channel])
                {
                    soundSystem->ReleaseFrame(soundFrame);
                }
            }
            break;
        }
        
        size_t idx = 0;
        while (idx < sFramesPerChannel[0]->mSample.max_size())
        {
            if (mVorbisIdx == mVorbisCount)
            {
                mVorbisIdx = 0;
                mVorbisCount = stb_vorbis_get_frame_float(mVorbis, nullptr, &mVorbisFrame);
            }
            if (mVorbisCount == 0)
            {
                ++musicDone;
                break;
            }
            musicDone = 0;
            for (; mVorbisIdx < mVorbisCount && idx < sFramesPerChannel[0]->mSample.max_size(); ++mVorbisIdx, ++idx)
            {
                for (size_t channel = 0; channel < sFramesPerChannel.size(); ++channel)
                {
                    const float value = mVorbisFrame[channel][mVorbisIdx];
                    sFramesPerChannel[channel]->mSample[idx] = value * mVolume;
                }

            }
        }
        if (0 < idx)
        {
            int alreadySubmitted = mSubmittedFrame;
            mSubmittedFrame += SoundFrame::sample_size;
            for (size_t channel = 0; channel < sFramesPerChannel.size(); ++channel)
            {
                if (SoundFrame* soundFrame = sFramesPerChannel[channel])
                {
                    soundFrame->mDelay = -alreadySubmitted;
                    soundFrame->mCounter = 0 == channel ? &mSubmittedFrame : nullptr; // we assume stereo will remain synchronized
                    soundSystem->SubmitFrame(soundFrame);
                    sFramesPerChannel[channel] = nullptr;
                }
            }
        }
        else
        {
            for (size_t channel = 0; channel < sFramesPerChannel.size(); ++channel)
            {
                if (SoundFrame* soundFrame = sFramesPerChannel[channel])
                {
                    soundSystem->ReleaseFrame(soundFrame);
                }
            }
        }

    }
}
#if GUI_DEBUG()
void MusicEntity::debug_GUI()
{
    ImGui::SliderFloat("Volume", &mVolume, 0.f, 1.f);
}
#endif

void MusicEntity::FreeResource()
{
    if (mVorbis)
    {
        stb_vorbis_close(mVorbis);
        mVorbis = nullptr;
    }
    if (mFile)
    {
        Global::platform()->CloseFile(mFile);
        mFile = nullptr;
    }
}
