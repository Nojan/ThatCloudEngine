#include "loopmanager.hpp"

#include "../root.hpp"
#include "../boundingbox.hpp"
#include "../camera.hpp"
#include "../freecam.hpp"
#include "../global.hpp"
#include "../game_entity.hpp"
#include "../game_system.hpp"
#include "../gjk.hpp"
#include "../meshRenderer.hpp"
#include "../renderableMesh.hpp"
#include "../physic_system.hpp"
#include "../select_system.hpp"
#include "../renderer_list.hpp"
#include "../rendering_system.hpp"
#include "../resource.hpp"
#include "../resourcefile.hpp"
#include "../resourcemesh.hpp"
#include "../resourcecache.hpp"
#include "../resourcemanager.hpp"
#include "../skybox.hpp"
#include "../types.hpp"
#include "../transform_system.hpp"

#include "../visualdebug.hpp"

#include "../opengl_includes.hpp"

#include <SDL.h>
#include <cassert>
#include <algorithm>
#include <array>

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
{
    ResourceCache* cache = Global::resourceManager()->Cache();
    mResources.push_back(cache->get_or_create<ResourceMesh>("cube"));
    mResources.push_back(cache->get_or_create<ResourceMesh>("diamond"));
    mResources.push_back(cache->get_or_create<ResourceMesh>("triangle"));
    mResources.push_back(cache->get_or_create<ResourceMesh>("sphere"));
    mResources.push_back(cache->get_or_create<ResourceMesh>("funnel"));
    mResources.push_back(cache->get_or_create<ResourceMesh>("plane"));

    {
        GameSystem* gameSystem = Global::gameSytem();
        gameSystem->createSystem<TransformSystem>();
        gameSystem->createSystem<PhysicSystem>();
        gameSystem->createSystem<SelectSystem>();
        gameSystem->createSystem<RenderingSystem>();
    }
}

LoopManager::~LoopManager()
{}

void LoopManager::ListRenderer(std::vector<std::shared_ptr<IRenderer>>& rendererList)
{
    RendererList* renderList = Global::rendererList();
    {
        std::shared_ptr<Skybox> renderer;
        renderer.reset(Skybox::GenerateCheckered());
        renderList->addRenderer(renderer.get());
        rendererList.push_back(renderer);
    }
    {
        std::shared_ptr<MeshRenderer> renderer = std::make_shared<MeshRenderer>();
        renderList->addRenderer(renderer.get());
        rendererList.push_back(renderer);
    }
}

void LoopManager::ListResources(std::vector<Resource *> &resources)
{
    resources.reserve(resources.size() + mResources.size());
    for (auto& r : mResources)
    {
        resources.push_back(r.get());
    }
}

void LoopManager::OnLoad()
{
}

void LoopManager::Init()
{
    GameSystem* gameSystem = Global::gameSytem();
    Camera* camera = Root::Instance().GetCamera();
    std::unique_ptr<FreeCamera> cameraMover = std::make_unique<FreeCamera>();
    cameraMover->mPosition = -5.0f * Camera::forward + 2.0f * Camera::up;
    camera->SetCameraMover(std::move(cameraMover));

    {
        GameEntity* entity = gameSystem->createEntity();
        mEntities.push_back(entity);
        gameSystem->getSystem<TransformSystem>()->attachEntity(entity);
        TransformComponent* transform = entity->getComponent<TransformComponent>();
        transform->SetPosition(glm::vec4(0.f, 0.f, 0.f, 1.f));

        gameSystem->getSystem<RenderingSystem>()->attachEntity(entity);
        GraphicMeshComponent* renderingComponent = entity->getComponent<GraphicMeshComponent>();
        renderingComponent->mColor = { 0.f, 0.f, 1.f, 1.f };
        renderingComponent->setupResource(Global::resourceManager()->meshResource("../assets/plane.assxml"));

        gameSystem->getSystem<PhysicSystem>()->attachEntity(entity);
        PhysicComponent* physicComponent = entity->getComponent<PhysicComponent>();
        physicComponent->Reset();
        physicComponent->SetMass(0.f);

        //gameSystem->getSystem<SelectSystem>()->attachEntity(entity);
    }

    {
        GameEntity* entity = gameSystem->createEntity();
        mEntities.push_back(entity);
        gameSystem->getSystem<TransformSystem>()->attachEntity(entity);
        TransformComponent* transform = entity->getComponent<TransformComponent>();
        transform->SetPosition(glm::vec4(-1.f, 2.f, 0.f, 1.f));

        gameSystem->getSystem<RenderingSystem>()->attachEntity(entity);
        GraphicMeshComponent* renderingComponent = entity->getComponent<GraphicMeshComponent>();
        renderingComponent->mColor = { 0.f, 0.f, 1.f, 1.f };
        renderingComponent->setupResource(Global::resourceManager()->meshResource("../assets/cube.assxml"));

        gameSystem->getSystem<PhysicSystem>()->attachEntity(entity);
        PhysicComponent* physicComponent = entity->getComponent<PhysicComponent>();
        physicComponent->Reset();
        physicComponent->SetMass(1.f);

        gameSystem->getSystem<SelectSystem>()->attachEntity(entity);
    }

    if(false)
    {
        GameEntity* entity = gameSystem->createEntity();
        mEntities.push_back(entity);
        gameSystem->getSystem<TransformSystem>()->attachEntity(entity);
        TransformComponent* transform = entity->getComponent<TransformComponent>();
        transform->SetPosition(glm::vec4(-1.f, 4.f, 0.f, 1.f));

        gameSystem->getSystem<RenderingSystem>()->attachEntity(entity);
        GraphicMeshComponent* renderingComponent = entity->getComponent<GraphicMeshComponent>();
        renderingComponent->mColor = { 0.f, 0.f, 1.f, 1.f };
        renderingComponent->setupResource(Global::resourceManager()->meshResource("../assets/cube.assxml"));

        gameSystem->getSystem<PhysicSystem>()->attachEntity(entity);
        PhysicComponent* physicComponent = entity->getComponent<PhysicComponent>();
        physicComponent->Reset();
        physicComponent->SetMass(1.f);

        gameSystem->getSystem<SelectSystem>()->attachEntity(entity);
    }

    if(false)
    {
        GameEntity* entity = gameSystem->createEntity();
        mEntities.push_back(entity);
        gameSystem->getSystem<TransformSystem>()->attachEntity(entity);
        TransformComponent* transform = entity->getComponent<TransformComponent>();
        transform->SetPosition(glm::vec4(-4.f, 3.5f, 0.f, 1.f));

        gameSystem->getSystem<RenderingSystem>()->attachEntity(entity);
        GraphicMeshComponent* renderingComponent = entity->getComponent<GraphicMeshComponent>();
        renderingComponent->mColor = { 0.f, 0.f, 1.f, 1.f };
        renderingComponent->setupResource(Global::resourceManager()->meshResource("../assets/diamond.assxml"));

        gameSystem->getSystem<SelectSystem>()->attachEntity(entity);
    }

    if (false)
    {
        GameEntity* entity = gameSystem->createEntity();
        mEntities.push_back(entity);
        gameSystem->getSystem<TransformSystem>()->attachEntity(entity);
        TransformComponent* transform = entity->getComponent<TransformComponent>();
        transform->SetPosition(glm::vec4(-6.f, 3.5f, 0.f, 1.f));

        gameSystem->getSystem<RenderingSystem>()->attachEntity(entity);
        GraphicMeshComponent* renderingComponent = entity->getComponent<GraphicMeshComponent>();
        renderingComponent->mColor = { 0.f, 0.f, 1.f, 1.f };
        renderingComponent->setupResource(Global::resourceManager()->meshResource("../assets/triangle.assxml"));

        gameSystem->getSystem<SelectSystem>()->attachEntity(entity);
    }


}

void LoopManager::Terminate()
{

}

struct PhysShapeCollider {
    GameEntity* entity = nullptr;
    std::unique_ptr<PhysShape> shape;
    BoundingBox3D bbox;
    glm::mat4 transform;
};

struct PhysManifold {
    uint16_t bodyA = -1;
    uint16_t bodyB = -1;

    float distance = 0.f;
};

class PhysMeshShape : public PhysConvexShape {
public:
    std::vector<uint> mIndex;
};

void LoopManager::FrameStep()
{
    const Camera* camera = Root::Instance().GetCamera();
    const glm::vec3& position = camera->Position();
    const glm::vec3& camera_direction = camera->MouseDirection();

    GameSystem* gameSystem = Global::gameSytem();
    gameSystem->getSystem<SelectSystem>()->SelectWithRay(position, position + camera_direction * 100.f);

    if (mSpacePressed)
    {
        if (GameEntity* selected = gameSystem->getSystem<SelectSystem>()->GetSelected())
        {
            TransformComponent* transform = selected->getComponent<TransformComponent>();
            const glm::vec3 selectedPos = glm::vec3(transform->Position());
            const glm::vec3 selectedNormal = -camera->Direction();

            const float tdn = glm::dot(selectedNormal, camera_direction);
            if (0.0001f < fabsf(tdn))
            {
                //const float d = glm::length(selectedPos - position);
                const float t = - glm::dot(selectedNormal, position) / tdn;
                if (0.f < t)
                {
                    const glm::vec3 newPosition = position + t * camera_direction;
                    transform->SetPosition(glm::vec4(newPosition, 1.f));
                    if (PhysicComponent* physics = selected->getComponent<PhysicComponent>())
                    {
                        physics->SetLinearVelocity(glm::vec4(0.f));
                        physics->SetAngularVelocity(glm::vec4(0.1f, 0.1f, 0.1f, 0.f)); // TMP add some rotation to test the collision system
                    }
                }
            }
        }
    }

    // TODO: move everything into physics system
    std::vector<PhysShapeCollider> colliders;
    for (GameEntity* entity : mEntities)
    {
        TransformComponent* transform = entity->getComponent<TransformComponent>();
        GraphicMeshComponent* renderingComponent = entity->getComponent<GraphicMeshComponent>();
        if(!transform || !renderingComponent)
            continue;
        colliders.emplace_back();
        PhysShapeCollider& collider = colliders.back();
        collider.entity = entity;
        collider.transform = transform->Transform();
        collider.bbox = renderingComponent->getLocalBoundingBox();
        auto findVertice = [](const std::vector<glm::vec3>& collection, const glm::vec3& vertex) -> size_t
        {
            size_t result = -1;
            for(size_t idx = 0; idx < collection.size(); ++idx)
            {
                const glm::vec3 diff = collection[idx] - vertex;
                const float magnitude = glm::dot(diff, diff);
                if (magnitude < 0.0001f)
                {
                    result = idx;
                    break; 
                }
            }
            return result;
        };
        if (true || entity != mEntities.back())
        {
            std::unique_ptr<PhysConvexShape> shape = std::make_unique<PhysConvexShape>();
            for (const std::shared_ptr<RenderableMesh>& mesh : renderingComponent->mRenderable)
            {
                for (const glm::vec3 vertex : mesh->mMesh->mVertex)
                {
                    if (size_t(-1) == findVertice(shape->mVertices, vertex))
                    {
                        shape->mVertices.push_back(vertex);
                    }
                }
            }
            collider.shape = std::move(shape);
        }
        else
        {
            std::unique_ptr<PhysMeshShape> shape = std::make_unique<PhysMeshShape>();
            for (const std::shared_ptr<RenderableMesh>& mesh : renderingComponent->mRenderable)
            {
                const uint startIndex = shape->mIndex.size();
                for (const glm::vec3 vertex : mesh->mMesh->mVertex)
                {
                    shape->mVertices.push_back(vertex);
                }
                for (const uint index : mesh->mMesh->mIndex)
                {
                    shape->mIndex.push_back(startIndex + index);
                }
            }
            collider.shape = std::move(shape);
        }
    }

#if 1
    PhysicSystem* physicSystem = gameSystem->getSystem<PhysicSystem>();
    for (size_t i = 0; i < colliders.size(); ++i)
    {
        const PhysShapeCollider& colliderA = colliders[i];
        for (size_t j = i + 1; j < colliders.size(); ++j)
        {
            const PhysShapeCollider& colliderB = colliders[j];
            const GjkInput input = GjkMakeInput(*colliderA.shape, *colliderB.shape, colliderA.transform, colliderB.transform);
            const GjkContact c = GJKComputeContact(input);
            const PhysicComponent::ContactManifold contact(c.distance, c.normal, c.position, colliderA.entity->getComponent<PhysicComponent>(), colliderB.entity->getComponent<PhysicComponent>());
            physicSystem->CreateContact(contact);
        }
    }
#else
    // generate perfect contact points without GJK and contact caching.
    // This validate the collision procedure
    if(!colliders.empty())
    {
        const float plane_height = 0.0580530018f;
        const PhysShapeCollider& collider = colliders.front();
        if (const PhysConvexShape* shape = dynamic_cast<const PhysConvexShape*>(collider.shape.get()))
        {
            if (PhysicComponent* physicComponent = collider.entity->getComponent<PhysicComponent>())
            {
                const glm::mat4 transform = physicComponent->mTransformComponent->Transform();
                physicComponent->mContacts.clear();
                for (const glm::vec3& v : shape->mVertices)
                {
                    glm::vec3 pointA = glm::vec3(transform * glm::vec4(v, 1.f));
                    glm::vec3 pointB = pointA;
                    pointB[1] = plane_height;
                    PhysicComponent::ContactManifold contact(pointA, pointB);
                    physicComponent->mContacts.push_back(contact);
                }
                if (4 < physicComponent->mContacts.size())
                {
                    auto sortByDistance = [](const PhysicComponent::ContactManifold& a, const PhysicComponent::ContactManifold& b) -> bool
                    {
                        return a.distance < b.distance;
                    };
                    std::sort(physicComponent->mContacts.begin(), physicComponent->mContacts.end(), sortByDistance);
                    physicComponent->mContacts.resize(4);
                }
            }
            
        }
    }
#endif
}

void LoopManager::Update(const float deltaTime)
{
}

void LoopManager::OnPhysicsEvent(PhysicEvent& e)
{
}

void LoopManager::Event(const SDL_Event& e)
{
    if (SDL_KEYDOWN == e.type && SDLK_SPACE == e.key.keysym.sym)
    {
        mSpacePressed = true;
    }
    if (SDL_KEYUP == e.type && SDLK_SPACE == e.key.keysym.sym)
    {
        mSpacePressed = false;
    }
}

void Gameplay::LoopManager::Control(const InputControl& input)
{

}

#if GUI_DEBUG()
void LoopManager::debug_GUI()
{

}
#endif

} //namespace Gameplay
