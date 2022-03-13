#include "loopmanager.hpp"

#include "../root.hpp"
#include "../boundingbox.hpp"
#include "../camera.hpp"
#include "../collider_resource.hpp"
#include "../input_control.hpp"
#include "../cameramover.hpp"
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

class CameraOrbitEntity : public CameraMover {
public:
    void Move(const float speed, Camera* camera) override;
    void Control(const InputControl& control, Camera* camera) override;

    void GetTransform(glm::vec3& position, glm::quat& orientation) override;

    TransformComponent* mFollow = nullptr;
    glm::vec3 mOrbitPosition = glm::vec3(0, 0, 0);
    glm::vec2 mMousePosition = glm::vec2(0, 0);
    glm::vec2 mEulerAngle = glm::vec2(0, 0);
    float mDistance = 5.f;
};

void CameraOrbitEntity::Move(const float speed, Camera* camera)
{
    if (mFollow)
        camera->SetPosition(glm::vec3(mFollow->Position()) - camera->Direction() * mDistance);
}

void CameraOrbitEntity::Control(const InputControl& control, Camera* camera)
{
    bool positionChanged = false;
    if (mFollow)
    {
        const glm::vec3 followPosition(mFollow->Position());
        if (followPosition != mOrbitPosition)
        {
            positionChanged = true;
            mOrbitPosition = followPosition;
        }
    }
    if (0 != control.zoom)
    {
        mDistance = glm::clamp(mDistance - control.zoom, 5.f, 500.f);
        positionChanged = true;
    }
    bool orientationChanged = false;
    if (0.f != glm::dot(control.view, control.view))
    {
        mEulerAngle.x -= control.view.y;
        mEulerAngle.y += control.view.x;
        if (mEulerAngle.x < glm::pi<float>())
            mEulerAngle.x += 2.f * glm::pi<float>();
        if (mEulerAngle.x > glm::pi<float>())
            mEulerAngle.x -= 2.f * glm::pi<float>();
        if (mEulerAngle.y < glm::pi<float>())
            mEulerAngle.y += 2.f * glm::pi<float>();
        if (mEulerAngle.y > glm::pi<float>())
            mEulerAngle.y -= 2.f * glm::pi<float>();
    
        orientationChanged = true;
    }
    if (orientationChanged)
    {
        const glm::quat r = glm::normalize(glm::quat(glm::vec3(mEulerAngle, 0.f)));
        camera->SetOrientation(r);
        positionChanged = true;
    }
    if (positionChanged)
    {
        const glm::vec3 position = mOrbitPosition - camera->Direction() * mDistance;
        camera->SetPosition(position);
    }
}

void CameraOrbitEntity::GetTransform(glm::vec3& position, glm::quat& orientation)
{
    orientation = glm::normalize(glm::quat(glm::vec3(mEulerAngle, 0.f)));
    position = mOrbitPosition - orientation * Camera::forward * mDistance;
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
    mResources.push_back(cache->get_or_create<ResourceMesh>("world"));

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
    std::unique_ptr<CameraOrbitEntity> cameraMover = std::make_unique<CameraOrbitEntity>();
    cameraMover->mOrbitPosition = -5.0f * Camera::forward + 2.0f * Camera::up;
    mCamera = cameraMover.get();
    camera->SetCameraMover(std::move(cameraMover));

    std::array<const Mesh*, 8> colliderMesh;
    auto BuildColliderFromRenderable = [&colliderMesh](GraphicMeshComponent* renderingComponent, PhysicComponent* physicComponent)
    {
        ColliderDescriptor descriptor;
        descriptor.mesh = std::span(colliderMesh.begin(), std::min(colliderMesh.size(), renderingComponent->mRenderable.size()));
        descriptor.asConvex = physicComponent->HasFiniteMass();
        assert(renderingComponent->mRenderable.size() < colliderMesh.size());
        for (size_t idx = 0, endIdx = descriptor.mesh.size(); idx < endIdx; idx++)
        {
            colliderMesh[idx] = renderingComponent->mRenderable[idx]->mMesh.get();
        }
        physicComponent->mCollider = MakeCollider(descriptor);
    };

    {
        GameEntity* entity = gameSystem->createEntity();
        mEntities.push_back(entity);
        gameSystem->getSystem<TransformSystem>()->attachEntity(entity);
        TransformComponent* transform = entity->getComponent<TransformComponent>();
        transform->SetPosition(glm::vec4(0.f, 15.f, 0.f, 1.f));

        gameSystem->getSystem<RenderingSystem>()->attachEntity(entity);
        GraphicMeshComponent* renderingComponent = entity->getComponent<GraphicMeshComponent>();
        renderingComponent->mColor = { 0.f, 0.f, 1.f, 1.f };
        renderingComponent->setupResource(Global::resourceManager()->meshResource("../assets/cube.assxml"));

        gameSystem->getSystem<PhysicSystem>()->attachEntity(entity);
        PhysicComponent* physicComponent = entity->getComponent<PhysicComponent>();
        physicComponent->Reset();
        physicComponent->SetRadius(1.f);
        physicComponent->SetMass(1.f);

        BuildColliderFromRenderable(renderingComponent, physicComponent);

        gameSystem->getSystem<SelectSystem>()->attachEntity(entity);
        mPlayer = entity;
        mCamera->mFollow = transform;
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
        renderingComponent->setupResource(Global::resourceManager()->meshResource("../assets/world.assxml"));

        gameSystem->getSystem<PhysicSystem>()->attachEntity(entity);
        PhysicComponent* physicComponent = entity->getComponent<PhysicComponent>();
        physicComponent->Reset();
        physicComponent->SetMass(0.f);

        BuildColliderFromRenderable(renderingComponent, physicComponent);

        //gameSystem->getSystem<SelectSystem>()->attachEntity(entity);
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
    PhysShape* shape = nullptr;
    BoundingBox3D bbox;
    glm::mat4 transform;
};

struct PhysManifold {
    uint16_t bodyA = -1;
    uint16_t bodyB = -1;

    float distance = 0.f;
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
        PhysicComponent* physicComponent = entity->getComponent<PhysicComponent>();
        if (!physicComponent)
            continue;
        colliders.emplace_back();
        PhysShapeCollider& collider = colliders.back();
        collider.entity = entity;
        collider.transform = transform->Transform();
        collider.bbox = renderingComponent->getLocalBoundingBox();

        if (physicComponent->mCollider)
        {
            collider.shape = physicComponent->mCollider.get();
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
            if (const PhysMeshShape* meshShape = dynamic_cast<const PhysMeshShape*>(colliderB.shape))
            {
                PhysConvexShape triangleShape;
                triangleShape.mVertices.resize(3);
                uint bodyKey = 0;
                for (size_t idx = 0; (idx + 2) < meshShape->mIndex.size(); idx+=3)
                {
                    bodyKey++;

                    triangleShape.mVertices[0] = meshShape->mVertices[meshShape->mIndex[idx + 0]];
                    triangleShape.mVertices[1] = meshShape->mVertices[meshShape->mIndex[idx + 1]];
                    triangleShape.mVertices[2] = meshShape->mVertices[meshShape->mIndex[idx + 2]];

                    const GjkInput input = GjkMakeInput(*colliderA.shape, triangleShape, colliderA.transform, colliderB.transform);
                    const GjkContact c = GJKComputeContact(input);
                    PhysicComponent::ContactManifold contact(c.distance, c.normal, c.position, colliderA.entity->getComponent<PhysicComponent>(), colliderB.entity->getComponent<PhysicComponent>());

                    contact.bodyBKey = bodyKey;
                    physicSystem->CreateContact(contact);
                }
            }
            else
            {
                const GjkInput input = GjkMakeInput(*colliderA.shape, *colliderB.shape, colliderA.transform, colliderB.transform);
                const GjkContact c = GJKComputeContact(input);
                const PhysicComponent::ContactManifold contact(c.distance, c.normal, c.position, colliderA.entity->getComponent<PhysicComponent>(), colliderB.entity->getComponent<PhysicComponent>());
                physicSystem->CreateContact(contact);
            }
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
    if(!mPlayer)
        return;
    if (!mCamera)
        return;
    PhysicComponent* playerPhysic = mPlayer->getComponent<PhysicComponent>();
    
    glm::vec3 cameraPosition;
    glm::quat cameraRotation;
    mCamera->GetTransform(cameraPosition, cameraRotation);

    const glm::vec3 cameraForward = glm::normalize(cameraRotation * Camera::forward);
    const glm::vec3 cameraRight = glm::normalize(cameraRotation * Camera::right);

    const glm::vec3 forward = glm::normalize(cameraForward - glm::dot(cameraForward, Camera::up) * Camera::up);
    const glm::vec3 right = glm::normalize(cameraRight - glm::dot(cameraRight, Camera::up) * Camera::up);
    const float moveMag = glm::dot(input.move, input.move);
    if (0.1f < moveMag)
    {
        const glm::vec3 moveLocal = input.move.x * right + input.move.y * forward * -1.f;
        playerPhysic->AddForce(moveLocal * 5.f);
    }

    if (bool showDebug = false)
    {
        const glm::vec3 position(playerPhysic->GetTransform()[3]);
        VisualDebugRenderer* visualDebug = VisualDebug();
        auto drawArrow = [visualDebug](const glm::vec3& begin, const glm::vec3& end, const Color::rgbap& color, const float radius = 0.1f, const float ratio = 0.8f)
        {
            const glm::vec3 dir = end - begin;
            const float mag = glm::length(dir);
            if (mag == 0.f)
                return;
            const glm::vec3 normal = dir / mag;
            visualDebug->PushCommand(VisualDebugSegmentCommand(begin, end, color));
            visualDebug->PushCommand(VisualDebugHalfCone(begin + (normal * mag * ratio), end, radius, 0.f, color));
        };
        drawArrow(position, position + forward, { 0, 1, 0, 1 });
        drawArrow(position, position + right, { 1, 0, 0, 1 });
    }
}

#if GUI_DEBUG()
void LoopManager::debug_GUI()
{

}
#endif

} //namespace Gameplay
