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
        transform->SetPosition(glm::vec4(-1.f, 3.f, 0.f, 1.f));

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

        //gameSystem->getSystem<SelectSystem>()->attachEntity(entity);
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

bool TestIntersection(const PhysShapeCollider* colliderA, const PhysShapeCollider* colliderB, glm::vec3* pointA, glm::vec3* pointB)
{
    const PhysMeshShape* meshShapeA = dynamic_cast<const PhysMeshShape*>(colliderA->shape.get());
    const PhysMeshShape* meshShapeB = dynamic_cast<const PhysMeshShape*>(colliderB->shape.get());
    if (!meshShapeA && !meshShapeB)
    {
        GjkInput input = GjkMakeInput(*colliderA->shape, *colliderB->shape, colliderA->transform, colliderB->transform);
        GjkSimplex simplex;
        const bool result = GJkTestIntersection(input, simplex);
        if (!result)
        {
            GJKClosestPointOnShape(input, simplex, *pointA, *pointB);
        }
        return result;
    }
    else if (meshShapeA && meshShapeB)
    {

    }
    else
    {
        if (meshShapeB)
        {
            std::swap(meshShapeA, meshShapeB);
            std::swap(colliderA, colliderB);
            std::swap(pointA, pointB);
        }
        float shortestDistance = FLT_MAX;
        for (size_t idx = 0; idx < meshShapeA->mIndex.size(); idx += 3)
        {
            const uint vertexIdx[3] = { meshShapeA->mIndex[idx + 0], meshShapeA->mIndex[idx + 1], meshShapeA->mIndex[idx + 2] };

            PhysConvexShape subMesh;
            subMesh.mVertices.push_back(meshShapeA->mVertices[vertexIdx[0]]);
            subMesh.mVertices.push_back(meshShapeA->mVertices[vertexIdx[1]]);
            subMesh.mVertices.push_back(meshShapeA->mVertices[vertexIdx[2]]);
            GjkInput input = GjkMakeInput(subMesh, *colliderB->shape, colliderA->transform, colliderB->transform);
            GjkSimplex simplex;
            if (GJkTestIntersection(input, simplex))
            {
               return true;
            }
            glm::vec3 subPointA, subPointB;
            GJKClosestPointOnShape(input, simplex, subPointA, subPointB);
            const glm::vec3 diff = subPointB - subPointA;
            const float distance = glm::dot(diff, diff);
            if (distance < shortestDistance)
            {
                shortestDistance = distance;
                *pointA = subPointA;
                *pointB = subPointB;
            }
        }
        return false;
    }
    return true;
}

static void DrawContactManifold(const PhysicComponent::ContactManifold& contact)
{
    const Color::rgbap color = { 1, 1, 1, 1 };
    VisualDebugRenderer* visualDebug = VisualDebug();
    visualDebug->PushCommand(VisualDebugSegmentCommand(contact.position, contact.position + contact.normal, color));
    visualDebug->PushCommand(VisualDebugHalfCone(contact.position + (contact.normal * 0.8f), contact.position + contact.normal, 0.1f, 0.f, color));
}

int FindBestMatch(const PhysicComponent::ContactManifold& contact, const std::vector<PhysicComponent::ContactManifold>& collections)
{
    int result = -1;
    float bestMatch = 0.01f;
    auto computeMatch = [](const PhysicComponent::ContactManifold& a, const PhysicComponent::ContactManifold& b) -> float
    {
        const glm::vec3 diff = a.position - b.position;
        return glm::dot(diff, diff);// + fabsf(1.f - glm::dot(a.normal, b.normal));
    };
    for (int idx = 0, endIdx = numeric_cast<int>(collections.size()); idx < endIdx; ++idx)
    {
        const PhysicComponent::ContactManifold& candidate = collections[idx];
        const float match = computeMatch(contact, candidate);
        if (match < bestMatch)
        {
            bestMatch = match;
            result = idx;
        }
    }
    return result;
}

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
                        physics->mContacts.clear();
                    }
                }
            }
        }
    }

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
        if (entity != mEntities.back())
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

    for (size_t i = 0; i < colliders.size(); ++i)
    {
        const PhysShapeCollider& colliderA = colliders[i];
        for (size_t j = i + 1; j < colliders.size(); ++j)
        {
            const PhysShapeCollider& colliderB = colliders[j];
            glm::vec3 pointA, pointB;
            if (TestIntersection(&colliderA, &colliderB, &pointA, &pointB))
            {
                VisualDebugBoundingBoxCommand commandA(colliderA.bbox, { 0, 1, 0, 1 }, colliderA.transform, true);
                VisualDebugBoundingBoxCommand commandB(colliderB.bbox, { 0, 1, 0, 1 }, colliderB.transform, true);
                VisualDebug()->PushCommand(commandA);
                VisualDebug()->PushCommand(commandB);
            }
            else
            {
                auto sortContact = [](const PhysicComponent::ContactManifold& a, const PhysicComponent::ContactManifold& b) -> bool
                {
                    return a.distance < b.distance;
                };
                auto triangleAreaEstimate = [](const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) -> float
                {
                    const glm::vec3 ab = b - a;
                    const glm::vec3 ac = c - a; 
                    const glm::vec3 cross = glm::cross(ab, ac);
                    const float parallelogrameAreaSquared = glm::dot(cross, cross);
                    return parallelogrameAreaSquared;
                };
                auto addContactToPhysicComponent = [&](std::vector<PhysicComponent::ContactManifold>& contacts, const PhysicComponent::ContactManifold& contact)
                {
                    const int bestMatchIdx = FindBestMatch(contact, contacts);
                    if (0 <= bestMatchIdx)
                    {
                        contacts[bestMatchIdx] = contact;
                    }
                    else
                    {
                        if (3 < contacts.size())
                        {
                            std::sort(contacts.begin(), contacts.end(), sortContact);
                            const float currentArea = triangleAreaEstimate(contacts[1].position, contacts[2].position, contacts[3].position);
                            int bestIdx = -1;
                            for (int cIdx = 1; cIdx <= 3; ++cIdx)
                            {
                                const float area = triangleAreaEstimate(1 == cIdx ? contact.position : contacts[1].position, 2 == cIdx ? contact.position : contacts[2].position, 3 == cIdx ? contact.position : contacts[3].position);
                                if (currentArea < area)
                                {
                                    bestIdx = cIdx;
                                }
                                if (0 < bestIdx)
                                {
                                    contacts[bestIdx] = contact;
                                }
                            }
                        }
                        else
                            contacts.push_back(contact);
                    }
                };

                if (PhysicComponent* physicComponent = colliderA.entity->getComponent<PhysicComponent>())
                {
                    PhysicComponent::ContactManifold contact(pointA, pointB);
                    addContactToPhysicComponent(physicComponent->mContacts, contact);
                }
                if (PhysicComponent* physicComponent = colliderB.entity->getComponent<PhysicComponent>())
                {
                    PhysicComponent::ContactManifold contact(pointB, pointA);
                    addContactToPhysicComponent(physicComponent->mContacts, contact);
                }
                
                VisualDebugSegmentCommand command(pointA, pointB, { 0, 1, 0, 1 });
                VisualDebug()->PushCommand(command);
                VisualDebugSphereCommand sphere(pointB, 0.01f, { 0, 1, 0, 1 });
                VisualDebug()->PushCommand(sphere);
                
            }
        }
    }
    for (size_t i = 0; i < colliders.size(); ++i)
    {
        if (PhysicComponent* physicComponent = colliders[i].entity->getComponent<PhysicComponent>())
        {
            for (size_t i = 0; i < physicComponent->mContacts.size(); ++i)
            {
                const PhysicComponent::ContactManifold& contact = physicComponent->mContacts[i];
                DrawContactManifold(contact);
            }
        }
    }
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
