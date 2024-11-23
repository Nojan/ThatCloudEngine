#include "game_system.hpp"
#include "icomponentsystem.hpp"

#include "billboard_rendering_system.hpp"
#include "physic_system.hpp"
#include "transform_system.hpp"
#include "rendering_system.hpp"
#include "animated_texture_system.hpp"
#include "sound_system.hpp"

#include "imgui/imgui_header.hpp"
#include "tracy_helper.hpp"
#include <cassert>

GameSystem::GameSystem()
{
}

GameSystem::~GameSystem()
{
    while (!mEntities.empty())
    {
        removeEntitySync(mEntities.front().get());
    }
    mEntities.clear();
    mSystems.clear();
}

void GameSystem::Update(const float deltaTime)
{
    ZoneScopedN("GameSystem::Update");
    for (auto& componentSystem : mSystems)
    {
        componentSystem->Update(deltaTime);
    }
}

void GameSystem::FrameStep()
{
    ZoneScopedN("GameSystem::Framestep");
    for (const auto& deadEntity : mDeadEntities)
    {
        removeEntitySync(deadEntity);
    }
    mDeadEntities.clear();
    for (auto& componentSystem : mSystems)
    {
        componentSystem->FrameStep();
    }
}

void GameSystem::addUntypedSystem(std::type_index index, void * untypedPointer)
{
    assert(nullptr == mSystemsMap[index]);
    mSystemsMap[index] = untypedPointer;
    assert(untypedPointer == mSystemsMap[index]);
}

void* GameSystem::getUntypedSystem(std::type_index index)
{
    return mSystemsMap[index];
}

GameEntity* GameSystem::createEntity()
{
    assert(GameEntity::Max > mEntities.size());
    std::unique_ptr<GameEntity> entity(new GameEntity());
    GameEntity* entityPtr = entity.get();
    mEntities.push_back(std::move(entity));
    return entityPtr;
}

void GameSystem::removeEntitySync(GameEntity* entity)
{
    assert(nullptr != entity);
    for (auto& componentSystem : mSystems)
    {
        componentSystem->detachEntity(entity);
    }

    const size_t entityCount = mEntities.size();
    assert(0 < entityCount);
    for (size_t i = 0; i<entityCount; ++i)
    {
        if (mEntities[i].get() == entity)
        {
            mEntities[i].reset();
            const size_t entityNewCount = entityCount - 1;
            std::swap(mEntities[i], mEntities[entityNewCount]);
            mEntities.resize(entityNewCount);
            break;
        }
    }
}

void GameSystem::removeEntity(GameEntity* entity)
{
    mDeadEntities.push_back(entity);
}

#if GUI_DEBUG()
void GameSystem::debug_GUI() const
{
    if (ImGui::CollapsingHeader("Game System"))
    {
        for (auto& componentSystem : mSystems)
        {
            if (ImGui::CollapsingHeader(componentSystem->debug_name()))            
                componentSystem->debug_GUI();
        }
    }
}
#endif
