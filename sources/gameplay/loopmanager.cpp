#include "loopmanager.hpp"

#include "boy.hpp"
#include "loadlevel.hpp"
#include "../root.hpp"
#include "../camera.hpp"
#include "../physic_system.hpp"
#include "../game_entity.hpp"
#include "../game_system.hpp"
#include "../global.hpp"
#include "../music_entity.hpp"
#include "../animated_texture_system.hpp"
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
{}

LoopManager::~LoopManager()
{}

void LoopManager::Init()
{
    mMusic->Init();
    mBoy->Init();

    char filename[256];
    std::vector<std::shared_ptr<Texture2D>> waveTextures;
    waveTextures.reserve(60);
    for (size_t i = 0; i < 60; ++i)
    {
        sprintf(filename, "../assets/3D/wave_5_%d.tga", i);
        waveTextures.push_back( Global::resourceManager()->texture(filename) );
    }

    GameSystem* gameSystem = Global::gameSytem();
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
    loadlevel(level_name, mEntities, boyPosition, cameraOffset);

    mBoy->TeleportTo(boyPosition);
    const float cameraDistance = cameraOffset.x;
    Camera* camera = Root::Instance().GetCamera();
    const glm::vec3 cameraPosition = boyPosition - (camera->Direction() * cameraDistance);
    camera->SetPosition(cameraPosition);
    
}

void LoopManager::Terminate()
{
    mMusic->Terminate();
    mBoy->Terminate();
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
    VisualDebug()->PushCommand(VisualDebugSphereCommand(intersect, 0.25f, {1.f, 0.f, 0.f, 1.f}));

    mBoy->MoveToward(intersect, deltaTime);
    
    // pull clouds toward the intersection
    if (mClickLeft)
    {
        for (auto& entity : mEntities)
        {
            PhysicComponent* physic = entity->getComponent<PhysicComponent>();
            if(!physic)
                continue;
            const glm::vec3 position(physic->mTransformComponent->Position());
            const glm::vec3 direction = intersect - position;
            const float distanceSq = glm::dot(direction, direction);
            const float limitSq = 4000.f;
            if (0 < distanceSq && distanceSq < limitSq)
            {
                const float distance = sqrt(distanceSq);
                const glm::vec4 normal(direction / distance, 0.f);
                physic->SetLinearVelocity(physic->LinearVelocity() + normal * 10.f);
            }
        }
    }
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
}

#ifdef IMGUI_ENABLE
void LoopManager::debug_GUI() const
{}
#endif

} //namespace Gameplay
