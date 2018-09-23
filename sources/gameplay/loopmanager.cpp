#include "loopmanager.hpp"

#include "../root.hpp"
#include "../camera.hpp"
#include "../game_entity.hpp"
#include "../game_system.hpp"
#include "../global.hpp"
#include "../music_entity.hpp"
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
{}

LoopManager::~LoopManager()
{}

void LoopManager::Init()
{
    mMusic->Init();

    GameSystem* gameSystem = Global::gameSytem();
    const char* meshes[] = {"islandvolcano", "cityvolcano", "islandsrest", "beachvolcano", "treevolcano"}; 

    for (size_t i = 0; i < 3; ++i)
    {
        GameEntity* entity = gameSystem->createEntity();
        mEntities.push_back(entity);
        gameSystem->getSystem<TransformSystem>()->attachEntity(entity);
        TransformComponent* transform = entity->getComponent<TransformComponent>();

        gameSystem->getSystem<RenderingSystem>()->attachEntity(entity);
        GraphicMeshComponent* renderingComponent = entity->getComponent<GraphicMeshComponent>();
        renderingComponent->mColor = { 0.f, 0.f, 1.f, 1.f };
        
        char filename[256];
        sprintf(filename, "../assets/3D/%s.assxml", meshes[i]);
        renderingComponent->setupResource( Global::resourceManager()->meshResource(filename) );
    }
}

void LoopManager::Terminate()
{
    mMusic->Terminate();
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
}

void LoopManager::Event(const SDL_Event & e)
{
    if (SDL_MOUSEBUTTONUP == e.type && SDL_BUTTON_LEFT == e.button.button)
    {
        const Camera* camera = Root::Instance().GetCamera();
        const glm::vec3& mouseDirection = camera->MouseDirection();
        const glm::vec3 planeNormal(0, 0, 1.f);
        const float cosTheta = glm::dot(mouseDirection, planeNormal);
        if (0.f == cosTheta)
            return;
        const glm::vec3& cameraPosition = camera->Position();
        const float planeDistance = 15.f;
        const float t = -(glm::dot(cameraPosition, planeNormal) + planeDistance) / cosTheta;
        const Camera::perspective& parameter = camera->Perspective();
        if (t < parameter.zNear || parameter.zFar < t)
            return;
        const glm::vec3 intersect = cameraPosition + mouseDirection*t;
    }
}

#ifdef IMGUI_ENABLE
void LoopManager::debug_GUI() const
{}
#endif

} //namespace Gameplay
