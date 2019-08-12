#include "loopmanager.hpp"

#include "boy.hpp"
#include "cloudsystem.hpp"
#include "gamecamera.hpp"
#include "gameconstant.hpp"
#include "gridsystem.hpp"
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
#include "../sound_system.hpp"
#include "../texture.hpp"
#include "../transform_system.hpp"

#include "../visualdebug.hpp"

#include "../opengl_includes.hpp"

#include <SDL2/SDL.h>
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
        gameSystem->createSystem<GridCellSystem>();
        gameSystem->getSystem<PhysicSystem>()->m_listener = this;
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
        SpawnCloud(cloudPosition, cloudColor, cloudPower);
    };
    GridSpawner gridSpawner = [this, gameSystem](const char* name, const int x, const int y)
    {
        ++mGridCount;
        glm::vec3 position(x, GridCellSystem::GetHeight(), y);
        
        GameEntity* entity = gameSystem->createEntity();
        mEntities.push_back(entity);
        gameSystem->getSystem<TransformSystem>()->attachEntity(entity);
        TransformComponent* transform = entity->getComponent<TransformComponent>();
        transform->SetPosition(glm::vec4(position, 1.f));

        gameSystem->getSystem<GridCellSystem>()->attachEntity(entity);
    };

    loadlevel(level_name, cloudSpawner, gridSpawner, boyPosition, cameraOffset);

    mBoy->TeleportTo(boyPosition);
    mCursor->SetPosition(boyPosition, 0.f);
    const float cameraDistance = cameraOffset.x;
    Camera* camera = Root::Instance().GetCamera();
    const glm::vec3 cameraPosition = boyPosition - (camera->Direction() * cameraDistance);
    camera->SetPosition(cameraPosition);
    std::unique_ptr<BoyCamera> cameraMover = std::make_unique<BoyCamera>();
    cameraMover->mBoy = mBoy.get();
    camera->SetCameraMover(std::move(cameraMover));

    {
        GameEntity* entity = gameSystem->createEntity();
        mSoundEffects.reset(entity);
        gameSystem->getSystem<SoundSystem>()->attachEntity(entity);
        SoundComponent* soundComponent = entity->getComponent<SoundComponent>();
        int soundEffectIdx;
        soundEffectIdx = soundComponent->AddResource( Global::resourceManager()->soundStream("../assets/Sounds/cloud_release.ogg") );
        assert(CloudRelease == soundEffectIdx);
        soundEffectIdx = soundComponent->AddResource( Global::resourceManager()->soundStream("../assets/Sounds/cloud_consume.ogg") );
        assert(CloudConsume == soundEffectIdx);
        soundEffectIdx = soundComponent->AddResource( Global::resourceManager()->soundStream("../assets/Sounds/cloud_normaltopurified.ogg") );
        assert(CloudNormalPurified == soundEffectIdx);
    }
}

void LoopManager::Terminate()
{
    GameSystem* gameSystem = Global::gameSytem();
    mMusic->Terminate();
    gameSystem->removeEntity(mSoundEffects.release());
    mBoy->Terminate();
    mCursor->Terminate();
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

    // update the grid
    mGridUpdateIdx = (mGridUpdateIdx + 1) % mGridCount;
    GridCellComponent* activeGrid = nullptr;
    int gridCloudCount = 0;
    for(int idx = numeric_cast<int>(mEntities.size()) - 1, gridIdx = 0; 0 <= idx; --idx)
    {
        GameEntity* entity = mEntities[idx];
        GridCellComponent* grid = entity->getComponent<GridCellComponent>();
        if(!grid)
            continue;
        if (gridIdx == mGridUpdateIdx)
        {
            activeGrid = grid;
            break;
        }
        ++gridIdx;
    }
    const BoundingBox3D gridBoundingBox = activeGrid ? activeGrid->GetBoundingBox() : BoundingBox3D();

    const Camera* camera = Root::Instance().GetCamera();
    const glm::vec3& mouseDirection = camera->MouseDirection();
    const glm::vec3 planeNormal(0, 1.f, 0);
    const float planeAltitude = Gameplay::grid_altitude;
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
    
    // Game update
    {
        GameEntity* closestCloud = nullptr;
        float closestCloudDistanceSq = FLT_MAX;
        int currentCloudCount = 0;
        const glm::vec3 boyPosition = mBoy->Position();
        const float cloudArea = glm::pi<float>() * cloud_radius * cloud_radius;
        mAdditionalRadius.SetTarget(sqrtf( numeric_cast<float>(mCloudCount) * cloudArea * glm::one_over_pi<float>() * 0.5));
        mAdditionalRadius.Update(deltaTime);
        const float addedDistance = mAdditionalRadius.GetValue();
        const float touchDistance = touch_distance + addedDistance;
        const float pullDistance = pull_distance + addedDistance;
        const float touchDistanceSq = touchDistance * touchDistance;
        for(int idx = numeric_cast<int>(mEntities.size()) - 1; 0 <= idx; --idx)
        {
            GameEntity* entity = mEntities[idx];
            CloudComponent* cloud = entity->getComponent<CloudComponent>();
            if(!cloud)
                continue;
            PhysicComponent* physic = entity->getComponent<PhysicComponent>();
            if(!physic)
                continue;
            const glm::vec3 position(physic->mTransformComponent->Position());
            if (GameDebugMode::None != mGameDebugMode)
            {
                VisualDebug()->PushCommand(VisualDebugCircleCommand(mBoy->Position(), glm::vec3(0.f, 1.f, 0.f), touchDistance, 48, {1.f, 1.f, 1.f, 1.f}));
                VisualDebug()->PushCommand(VisualDebugCircleCommand(mBoy->Position(), glm::vec3(0.f, 1.f, 0.f), touchDistance + pullDistance, 48, {1.f, 1.f, 1.f, 1.f}));
                if (GameDebugMode::Dot == mGameDebugMode)
                {
                    // Should a dot(constant size on screen)
                    VisualDebug()->PushCommand(VisualDebugSphereCommand(position, 1.0f, {1.f, 1.f, 1.f, 1.f}));
                }
                else
                {
                    const float radius = cloud_radius;
                    const glm::vec3 offset(0.0, 1.0, 0.0);
                    const glm::vec3 bottom = position - offset;
                    const glm::vec3 top = position + offset;
                    VisualDebug()->PushCommand(VisualDebugHalfCone(bottom, top, radius, radius, {1.f, 1.f, 1.f, 1.f}));
                }
            }
            if(0 == cloud->mColor)
                continue;

            if (gridBoundingBox.Inside(position))
            {
                ++gridCloudCount;
            }
            // pull clouds toward the boy
            if (mClickLeft)
            {
                assert(0.f < cloud->mPower);
                const glm::vec3 direction = boyPosition - position;
                const float distanceSq = glm::dot(direction, direction);
                const float limitSq = powf((touchDistance + pullDistance) * numeric_cast<float>(mCloudCount), 2.f);
                if (distanceSq < closestCloudDistanceSq)
                {
                    closestCloudDistanceSq = distanceSq;
                    closestCloud = entity;
                }
                if (distanceSq <= touchDistanceSq)
                {
                    currentCloudCount++;
                    if (mShiftLeft)
                    {
                        // absorb cloud
                        assert(0.f < cloud->mPower);
                        mStoredCloud += cloud->mPower;
                        GameSystem* gameSystem = Global::gameSytem();
                        gameSystem->removeEntity(entity);
                        const size_t lastIdx = mEntities.size() - 1;
                        std::swap(mEntities[idx], mEntities[lastIdx]);
                        mEntities.resize(lastIdx);
                        PlaySoundEffect(CloudConsume);
                    }
                }
                else if (touchDistanceSq < distanceSq && distanceSq < limitSq)
                {
                    const float distance = sqrt(distanceSq);
                    const glm::vec4 normal(direction / distance, 0.f);
                    physic->SetLinearVelocity(physic->LinearVelocity() + normal * pull_factor);
                }
            }
        }
        mCloudCount = currentCloudCount;
        // Spawn cloud
        if (mClickLeft && mCtrlLeft && 1.f <= mStoredCloud)
        {
            if (closestCloudDistanceSq < touchDistanceSq && nullptr != closestCloud)
            {
                const float addedPower = deltaTime;
                CloudComponent* cloud = closestCloud->getComponent<CloudComponent>();
                assert(cloud);
                BillboardComponent* billboardComponent = closestCloud->getComponent<BillboardComponent>();
                assert(billboardComponent);
                const int cloudCount = numeric_cast<int>(billboardComponent->mBillboards.size());
                Billboard& lastBillboard = billboardComponent->mBillboards.back();
                if (lastBillboard.mAlpha < 2.f)
                {
                    lastBillboard.mAlpha += addedPower;
                    cloud->mPower += addedPower;
                    mStoredCloud-= addedPower;
                }
                else
                {
                    mCloudTextureIdx = (mCloudTextureIdx + 1) % mCloudsTextures.size();
                    Billboard billboard;
                    billboard.mPosition = glm::vec3(0.f, float(cloudCount) * 5.f, 0.f);
                    billboard.mTexture = mCloudsTextures[mCloudTextureIdx];
                    billboard.mSize = glm::vec2(25.f);
                    billboard.mAlpha = addedPower;
                    billboardComponent->mBillboards.push_back(billboard);
                    cloud->mPower += addedPower;
                    mStoredCloud-= addedPower;
                }
            }
            else
            {
                PlaySoundEffect(CloudRelease);
                SpawnCloud(boyPosition, 1, 1.f);
                mStoredCloud-= 1.f;
            }
        }
    }

    if (activeGrid)
    {
        const bool isFilled = 0 < gridCloudCount;
        if (isFilled != activeGrid->mIsFilled)
        {
            if(isFilled)
                mGridFilled++;
            else
                mGridFilled--;
            activeGrid->mIsFilled = isFilled;
        }
    }
}

void LoopManager::OnPhysicsEvent(PhysicEvent & e)
{
    CloudComponent* aCloud = e.a->getComponent<CloudComponent>();
    CloudComponent* bCloud = e.b->getComponent<CloudComponent>();
    if (glm::vec4* ciVelocity = e.ciVelocity)
    {
        ciVelocity->y = 0.f;
    }
    if(nullptr == aCloud)
        return;
    if(nullptr == bCloud)
        return;
    if (aCloud->mColor != bCloud->mColor)
    {
        if(aCloud->mColor != 0)
        {
            std::swap(aCloud, bCloud);
            std::swap(e.a, e.b);
        }
        if(aCloud->mPower < bCloud->mPower)
        {
            aCloud->mColor = 1;
            BillboardComponent* billboard = e.a->getComponent<BillboardComponent>();
            for (int idx = billboard->mBillboards.size() - 1; 0 <= idx; idx--)
            {
                billboard->mBillboards[idx].mAlpha *= 2.f;
            }
            PlaySoundEffect(CloudNormalPurified);
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
    physic->SetRadius(cloud_radius);

    gameSystem->getSystem<BillboardRenderingSystem>()->attachEntity(entity);
    BillboardComponent* billboardComponent = entity->getComponent<BillboardComponent>();
    billboardComponent->mColor = { 0.f, 0.f, 1.f, 1.f };
    mCloudTextureIdx = (mCloudTextureIdx + 1) % mCloudsTextures.size();
    Billboard billboard;
    billboard.mPosition = glm::vec3(0.f);
    billboard.mTexture = mCloudsTextures[mCloudTextureIdx];
    billboard.mSize = glm::vec2(25.f);
    billboard.mAlpha = (0 < color) ? 2.f : 1.f;
    billboardComponent->mBillboards.push_back(billboard);

    gameSystem->getSystem<CloudSystem>()->attachEntity(entity);
    CloudComponent* cloudComponent = entity->getComponent<CloudComponent>();
    cloudComponent->mColor = color;
    cloudComponent->mPower = power;
}

void LoopManager::PlaySoundEffect(soundEffectIdx idx)
{
    SoundComponent* soundComponent = mSoundEffects->getComponent<SoundComponent>();
    SoundEffect* request = soundComponent->Play(idx);
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

    if (SDL_KEYDOWN == e.type && SDLK_r == e.key.keysym.sym)
    {
        GameSystem* gameSystem = Global::gameSytem();
        GridCellSystem* gridCellSystem = gameSystem->getSystem<GridCellSystem>();
        gridCellSystem->ShowDebugGrid(!gridCellSystem->GetDebugGrid());
    }

    if (SDL_KEYDOWN == e.type && SDLK_TAB == e.key.keysym.sym)
    {
        switch (mGameDebugMode)
        {
            case GameDebugMode::None:
                mGameDebugMode = GameDebugMode::Dot;
                for(int idx = numeric_cast<int>(mEntities.size()) - 1; 0 <= idx; --idx)
                {
                    GameEntity* entity = mEntities[idx];
                    CloudComponent* cloud = entity->getComponent<CloudComponent>();
                    if(!cloud)
                        continue;
                    BillboardComponent* billboardComponent = entity->getComponent<BillboardComponent>();
                    billboardComponent->mEnable = false;
                }
                break;
            case GameDebugMode::Dot:
                mGameDebugMode = GameDebugMode::Volume;
                for(int idx = numeric_cast<int>(mEntities.size()) - 1; 0 <= idx; --idx)
                {
                    GameEntity* entity = mEntities[idx];
                    CloudComponent* cloud = entity->getComponent<CloudComponent>();
                    if(!cloud)
                        continue;
                    BillboardComponent* billboardComponent = entity->getComponent<BillboardComponent>();
                    billboardComponent->mEnable = false;
                }
                break;
            case GameDebugMode::Volume:
                mGameDebugMode = GameDebugMode::None;
                for(int idx = numeric_cast<int>(mEntities.size()) - 1; 0 <= idx; --idx)
                {
                    GameEntity* entity = mEntities[idx];
                    CloudComponent* cloud = entity->getComponent<CloudComponent>();
                    if(!cloud)
                        continue;
                    BillboardComponent* billboardComponent = entity->getComponent<BillboardComponent>();
                    billboardComponent->mEnable = true;
                }
                break;
        }
    }
}

#ifdef IMGUI_ENABLE
void LoopManager::debug_GUI()
{
    ImGui::InputFloat("StoredCloud", &mStoredCloud, -100.f, 100.f);
    ImGui::Text("Grid %d/%d", mGridFilled, mGridCount);
    mMusic->debug_GUI();
#define ButtonSoundEffect(x)     \
if(ImGui::Button(#x))            \
{                                \
    PlaySoundEffect(x);          \
}                              

    if (ImGui::CollapsingHeader("Sound Effects"))
    {
        ButtonSoundEffect(CloudRelease);
        ButtonSoundEffect(CloudConsume);
        ButtonSoundEffect(CloudNormalPurified);
    }
#undef ButtonSoundEffect
}
#endif

} //namespace Gameplay
