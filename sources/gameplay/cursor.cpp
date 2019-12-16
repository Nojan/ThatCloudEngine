#include "cursor.hpp"

#include "../types.hpp"
#include "../global.hpp"
#include "../platform/platform.hpp"
#include "../game_entity.hpp"
#include "../game_system.hpp"

#include "../rendering_system.hpp"
#include "../billboard_rendering_system.hpp"
#include "../resourcecache.hpp"
#include "../resourcefile.hpp"
#include "../resourcemanager.hpp"
#include "../transform_system.hpp"

namespace Constant {
namespace Gameplay {
    constexpr float textureTimer =  0.1f;
}
}

Cursor::Cursor()
{
    ResourceCache* cache = Global::resourceManager()->Cache();
    char filename[256];
    mResources.reserve(12);
    for (int idx = 0; idx < 12; ++idx)
    {
        sprintf(filename, "../assets/3D/CursorBillboard_1_%d.tga", idx);
        mResources.push_back(cache->get_or_create<ResourceFile>(filename));
    }
}

void Cursor::ListResources(std::vector<Resource*>& resources)
{
    resources.reserve(resources.size() + mResources.size());
    for (auto& r : mResources)
    {
        resources.push_back(r.get());
    }
}

void Cursor::OnLoad()
{
    mTextures.clear();
    mTextures.reserve(12);
    for (auto& r : mResources)
    {
        mTextures.push_back(Global::resourceManager()->texture(r->name()));
    }
}

void Cursor::Init()
{
    GameSystem* gameSystem = Global::gameSytem();
    GameEntity* entity = gameSystem->createEntity();
    mEntity.reset(entity);

    gameSystem->getSystem<TransformSystem>()->attachEntity(entity);
    gameSystem->getSystem<BillboardRenderingSystem>()->attachEntity(entity);
    BillboardComponent* billboardComponent = entity->getComponent<BillboardComponent>();
    Billboard billboard;
    billboard.mPosition = glm::vec3(0.f);
    billboard.mSize = glm::vec2(1.f);
    billboard.mAlpha = 1.f;
    billboard.mTexture = mTextures[0];
    billboardComponent->mBillboards.push_back(billboard);
    billboardComponent->mColor = { 0.f, 0.f, 1.f, 1.f };

    mTimer = Constant::Gameplay::textureTimer;
}

void Cursor::Terminate()
{
    if (mEntity)
    {
        GameSystem* gameSystem = Global::gameSytem();
        gameSystem->removeEntity(mEntity.release());
    }
}

void Cursor::SetPosition(const glm::vec3 & position, const float deltaTime)
{
    TransformComponent* transform = mEntity->getComponent<TransformComponent>();
    transform->SetPosition(glm::vec4(position, 1.f));

    mTimer -= deltaTime;
    if (mTimer < 0.f)
    {
        mTimer = Constant::Gameplay::textureTimer;
        mIdx = (mIdx + 1) % mTextures.size();
        BillboardComponent* billboardComponent = mEntity->getComponent<BillboardComponent>();
        billboardComponent->mBillboards.front().mTexture = mTextures[mIdx];
    }
}
