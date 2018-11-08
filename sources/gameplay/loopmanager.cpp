#include "loopmanager.hpp"

#include "boy.hpp"
#include "cloudsystem.hpp"
#include "cursor.hpp"
#include "loadlevel.hpp"
#include "../root.hpp"
#include "../camera.hpp"
#include "../physic_system.hpp"
#include "../game_entity.hpp"
#include "../game_system.hpp"
#include "../global.hpp"
#include "../music_entity.hpp"
#include "../animated_texture_system.hpp"
#include "../billboard_rendering_system.hpp"
#include "../rendering_system.hpp"
#include "../renderableMesh.hpp"
#include "../resourcemanager.hpp"
#include "../texture.hpp"
#include "../transform_system.hpp"

#include "../visualdebug.hpp"

#include "../opengl_includes.hpp"

#include <SDL2/SDL.h>
#include <cassert>
#include <algorithm>

namespace Gameplay {

LoopManager::LoopManager()
: mMusic(new MusicEntity())
, mBoy(new Boy())
, mCursor(new Cursor())
{}

LoopManager::~LoopManager()
{}

void LoopManager::Init()
{
    GameSystem* gameSystem = Global::gameSytem();
    {
        gameSystem->createSystem<CloudSystem>();
    }
    
    mMusic->Init();
    mBoy->Init();
    mCursor->Init();

    char filename[256];
    mCloudsTextures.reserve(7);
    for (int idx = 1; idx <= 7; ++idx)
    {
        sprintf(filename, "../assets/3D/cloud_1_%d.tga", idx);
        mCloudsTextures.push_back( Global::resourceManager()->texture(filename) );
    }
    std::vector<std::shared_ptr<Texture2D>> waveTextures;
    waveTextures.reserve(60);
    for (size_t i = 0; i < 60; ++i)
    {
        sprintf(filename, "../assets/3D/wave_5_%d.tga", i);
        waveTextures.push_back( Global::resourceManager()->texture(filename) );
    }

    
    const char* meshes[] = {"islandvolcano", "cityvolcano", "islandsrest", "island3big", "oceanbottom_7", "shallowwater5volcano", "shallowwater5rest", "shallowwater4rest", "shallowwater43big", "ocean_3", "beachvolcano", "beachrest", "beach3big", "wavevolcano", "wave3big", "waverest", "treevolcano", "treerest", "tree3big" }; 

    for (size_t i = 0; i < sizeof(meshes)/sizeof(char*); ++i)
    {
        GameEntity* entity = gameSystem->createEntity();
        mEntities.push_back(entity);
        gameSystem->getSystem<TransformSystem>()->attachEntity(entity);
        TransformComponent* transform = entity->getComponent<TransformComponent>();

        gameSystem->getSystem<RenderingSystem>()->attachEntity(entity);
        GraphicMeshComponent* renderingComponent = entity->getComponent<GraphicMeshComponent>();
        renderingComponent->mColor = { 0.f, 0.f, 1.f, 1.f };
        
        sprintf(filename, "../assets/3D/%s.assxml", meshes[i]);
        renderingComponent->setupResource( Global::resourceManager()->meshResource(filename) );

        if (meshes[i] == strstr(meshes[i], "wave"))
        {
            gameSystem->getSystem<AnimatedTextureSystem>()->attachEntity(entity);
            AnimatedTextureComponent* animatedComponent = entity->getComponent<AnimatedTextureComponent>();
            animatedComponent->mTexture.insert(animatedComponent->mTexture.begin(), waveTextures.begin(), waveTextures.end());
        }
    }

    glm::vec3 boyPosition, cameraOffset;
    const char level_name[] = "../assets/Cloud/Levels/Yun.xml";
    CloudSpawner cloudSpawner = [this](const glm::vec3& cloudPosition, const int cloudColor, const float cloudPower)
    {
        this->SpawnCloud(cloudPosition, cloudColor, cloudPower);
    };
    GridSpawner gridSpawner = [this](const char* name, const int x, const int y){};
    loadlevel(level_name, cloudSpawner, gridSpawner, boyPosition, cameraOffset);

    mBoy->TeleportTo(boyPosition);
    mCursor->SetPosition(boyPosition, 0.f);
    const float cameraDistance = cameraOffset.x;
    Camera* camera = Root::Instance().GetCamera();
    const glm::vec3 cameraPosition = boyPosition - (camera->Direction() * cameraDistance);
    camera->SetPosition(cameraPosition);
    
}

void LoopManager::Terminate()
{
    mMusic->Terminate();
    mBoy->Terminate();
    mCursor->Terminate();
    GameSystem* gameSystem = Global::gameSytem();
    for (size_t idx = 0; idx < mEntities.size(); ++idx)
    {
        gameSystem->removeEntity(mEntities[idx]);
    }
    mEntities.clear();
}

void LoopManager::FrameStep()
{}

void LoopManager::Update(const float deltaTime)
{
    mMusic->Update(deltaTime);

    const Camera* camera = Root::Instance().GetCamera();
    const glm::vec3& mouseDirection = camera->MouseDirection();
    const glm::vec3 planeNormal(0, 1.f, 0);
    const float planeAltitude = 160.f;
    const float cosTheta = glm::dot(mouseDirection, planeNormal);
    if (0.f == cosTheta)
        return;
    const glm::vec3& cameraPosition = camera->Position();
    const float t = glm::dot( (planeNormal * planeAltitude) - cameraPosition, planeNormal) / cosTheta;
    const Camera::perspective& parameter = camera->Perspective();
    if (t < parameter.zNear || parameter.zFar < t)
        return;
    const glm::vec3 intersect = cameraPosition + mouseDirection*t;
    //VisualDebug()->PushCommand(VisualDebugSphereCommand(intersect, 0.25f, {1.f, 0.f, 0.f, 1.f}));

    mBoy->MoveToward(intersect, deltaTime);
    mCursor->SetPosition(intersect, deltaTime);
    
    // pull clouds toward the boy
    if (mClickLeft)
    {
        const glm::vec3 boyPosition = mBoy->Position();
        if (mCtrlLeft && 0 < mStoredCloud)
        {
            SpawnCloud(boyPosition, 1, 1.f);
            mStoredCloud--;
        }
        for(int idx = numeric_cast<int>(mEntities.size()) - 1; 0 <= idx; --idx)
        {
            GameEntity* entity = mEntities[idx];
            PhysicComponent* physic = entity->getComponent<PhysicComponent>();
            if(!physic)
                continue;
            const glm::vec3 position(physic->mTransformComponent->Position());
            const glm::vec3 direction = boyPosition - position;
            const float distanceSq = glm::dot(direction, direction);
            const float limitSq = 4000.f;
            if (mShiftLeft && distanceSq < 25.f)
            {
                // absorb cloud
                GameSystem* gameSystem = Global::gameSytem();
                gameSystem->removeEntity(entity);
                mStoredCloud++;
                const size_t lastIdx = mEntities.size() - 1;
                std::swap(mEntities[idx], mEntities[lastIdx]);
                mEntities.resize(lastIdx);
            }
            else if (0 < distanceSq && distanceSq < limitSq)
            {
                const float distance = sqrt(distanceSq);
                const glm::vec4 normal(direction / distance, 0.f);
                physic->SetLinearVelocity(physic->LinearVelocity() + normal * 10.f);
            }
        }
    }
}

void LoopManager::SpawnCloud(const glm::vec3& position, const int color, const float power)
{
    GameSystem* gameSystem = Global::gameSytem();
    GameEntity* entity = gameSystem->createEntity();
    mEntities.push_back(entity);
    gameSystem->getSystem<TransformSystem>()->attachEntity(entity);
    TransformComponent* transform = entity->getComponent<TransformComponent>();
    transform->SetPosition(glm::vec4(position, 1.f));
    gameSystem->getSystem<PhysicSystem>()->attachEntity(entity);
    PhysicComponent* physic = entity->getComponent<PhysicComponent>();

    gameSystem->getSystem<BillboardRenderingSystem>()->attachEntity(entity);
    BillboardComponent* billboardComponent = entity->getComponent<BillboardComponent>();
    billboardComponent->mColor = { 0.f, 0.f, 1.f, 1.f };
    mCloudTextureIdx = (mCloudTextureIdx + 1) % mCloudsTextures.size();
    billboardComponent->mBillboard.mTexture = mCloudsTextures[mCloudTextureIdx];
    billboardComponent->mBillboard.mSize = glm::vec2(25.f);
    billboardComponent->mBillboard.mAlpha = 1.f;

    gameSystem->getSystem<CloudSystem>()->attachEntity(entity);
    CloudComponent* cloudComponent = entity->getComponent<CloudComponent>();
    cloudComponent->mColor = color;
    cloudComponent->mPower = power;

    if(0 < color)
        billboardComponent->mBillboard.mAlpha = 2.f;
}

void LoopManager::Event(const SDL_Event & e)
{
    if (mClickLeft)
    {
        mClickLeft = !(SDL_MOUSEBUTTONUP == e.type && SDL_BUTTON_LEFT == e.button.button);
    }
    else
    {
        mClickLeft = (SDL_MOUSEBUTTONDOWN == e.type && SDL_BUTTON_LEFT == e.button.button);
    }

    if (mShiftLeft)
    {
        mShiftLeft = !(SDL_KEYUP == e.type && SDLK_LSHIFT == e.key.keysym.sym);
    }
    else
    {
        mShiftLeft = (SDL_KEYDOWN == e.type && SDLK_LSHIFT == e.key.keysym.sym);
    }

    if (mCtrlLeft)
    {
        mCtrlLeft = !(SDL_KEYUP == e.type && SDLK_LCTRL == e.key.keysym.sym);
    }
    else
    {
        mCtrlLeft = (SDL_KEYDOWN == e.type && SDLK_LCTRL == e.key.keysym.sym);
    }
}

#ifdef IMGUI_ENABLE
void LoopManager::debug_GUI() const
{}
#endif

} //namespace Gameplay
