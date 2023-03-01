#include "physic_system.hpp"

#include "transform_system.hpp"
#include "game_entity.hpp"
#include "types.hpp"

#include <tracy/Tracy.hpp>
#include <cassert>

PhysicComponent::PhysicComponent()
: mTransformComponent(nullptr)
, mEntity(nullptr)
, mInvMass(1)
, mForceAccum(0)
, mLinearVelocity(0,0,0,0)
, mLinearAcceleration(0,0,0,1)
, mAngularVelocity(0,0,0,0)
{}

PhysicComponent::PhysicComponent(const PhysicComponent& ref)
: mTransformComponent(ref.mTransformComponent)
, mEntity(ref.mEntity)
, mInvMass(ref.mInvMass)
, mRadius(ref.mRadius)
, mForceAccum(ref.mForceAccum)
, mLinearVelocity(ref.mLinearVelocity)
, mLinearAcceleration(ref.mLinearAcceleration)
{}

bool PhysicComponent::IsValid() const
{
    return nullptr != mTransformComponent;
}

bool PhysicComponent::HasFiniteMass() const
{
    return 0.f != mInvMass;
}

void PhysicComponent::SetMass(const float mass)
{
    assert(0 <= mass);
    if (0 == mass)
        mInvMass = FLT_MAX;
    else
        mInvMass = 1.f / mass;
}

void PhysicComponent::SetRadius(const float radius)
{
    assert(0.0f <= radius);
    mRadius = radius;
}

void PhysicComponent::Reset()
{
    mForceAccum = glm::vec3(0);
    mLinearVelocity = glm::vec4(0, 0, 0, 0);
    mLinearAcceleration = glm::vec4(0, 0, 0, 1);
    mAngularVelocity = glm::vec4(0, 0, 0, 0);
}

void PhysicComponent::Integrate(const float deltaTime)
{
    if (!IsValid() || !HasFiniteMass())
        return;

    const glm::vec4 force(mForceAccum, 0.f);
    mLinearAcceleration += force*mInvMass;
    assert(0.f == mLinearVelocity.w);
    mLinearVelocity += mLinearAcceleration*deltaTime;
    mLinearVelocity.w = 0.f;
    const glm::vec4 position = mTransformComponent->Position();
    assert(1.f == position.w);
    const glm::vec4 nextPosition = position + mLinearVelocity*deltaTime;
    mTransformComponent->SetPosition(nextPosition);

    const glm::quat& currentOrientation = mTransformComponent->Rotation();
    const glm::quat angularVelocityQuat(0, mAngularVelocity.x, mAngularVelocity.y, mAngularVelocity.z);
    const glm::quat spin = deltaTime * 0.5f * angularVelocityQuat * currentOrientation;
    const glm::quat newOrientation = currentOrientation + spin;
    mTransformComponent->SetRotation(glm::normalize(newOrientation));

    //Reset
    mForceAccum = glm::vec3(0.f);
    mLinearAcceleration = glm::vec4(0.f);
    //Drag
    const glm::vec4 drag(0.9999f);
    mLinearVelocity = mLinearVelocity * drag;
    mAngularVelocity = mAngularVelocity  * drag;
}

void PhysicComponent::AddForce(const glm::vec3& force)
{
    mForceAccum += force;
}

const glm::vec4& PhysicComponent::LinearVelocity() const
{
    return mLinearVelocity;
}

void PhysicComponent::SetLinearVelocity(const glm::vec4& velocity)
{
    mLinearVelocity = velocity;
}

const glm::vec4 & PhysicComponent::AngularVelocity() const
{
    return mAngularVelocity;
}

void PhysicComponent::SetAngularVelocity(const glm::vec4 & velocity)
{
    mAngularVelocity = velocity;
}

PhysicSystem::PhysicSystem()
{
    mComponents.reserve(GameEntity::Max);
}

PhysicSystem::~PhysicSystem()
{}

namespace SpatialAccel {
    struct KeyValue {
        uint key = ~0;
        uint value = ~0;
    };

    constexpr uint kHashTableCapacity = 1024;
    static_assert(sizeof(KeyValue) == 2 * sizeof(uint), "");
    constexpr uint kHashTableBufferSize = kHashTableCapacity * 2 * sizeof(uint);
    constexpr uint kEmpty = ~0;

    void HashTableFree(KeyValue* hashtable)
    {
        free(hashtable);
    }

    void HashTableClear(KeyValue* hashtable)
    {
        memset(hashtable, kEmpty, kHashTableBufferSize);
    }

    KeyValue* HashTableInit()
    {
        KeyValue* hashtable = static_cast<KeyValue*>(malloc(kHashTableBufferSize));
        HashTableClear(hashtable);
        return hashtable;
    }

    uint HashTableHash(uint key)
    {
        key ^= key >> 16;
        key *= 0x85ebca6b;
        key ^= key >> 13;
        key *= 0xc2b2ae35;
        key ^= key >> 16;
        return key & (kHashTableCapacity - 1);
    }

    void HashTableInsert(KeyValue* hashtable, const uint key, const uint value)
    {
        uint slot = HashTableHash(key);
        while (true)
        {
            uint prev = hashtable[slot].key;
            if (prev == kEmpty || prev == key)
            {
                hashtable[slot].key = key;
                hashtable[slot].value = value;
                break;
            }
            slot = (slot + 1) & (kHashTableCapacity - 1);
        }
    }

    uint HashTableLookup(KeyValue* hashtable, const uint key)
    {
        uint slot = HashTableHash(key);
        while (true)
        {
            if (hashtable[slot].key == key)
            {
                return hashtable[slot].value;
            }
            if (hashtable[slot].key == kEmpty)
            {
                return kEmpty;
            }
            slot = (slot + 1) & (kHashTableCapacity - 1);
        }
    }
};

glm::ivec3 ClampPosition3D(const glm::vec3& p, const float cellLengthInv)
{
    glm::ivec3 result;
    for (uint d = 0; d < 3; ++d)
    {
        result[d] = int(p[d] * cellLengthInv);
    }
    return result;
}

uint HashPosition3D(const glm::ivec3& p)
{
    const uint primes[3] = {73856093, 19349669, 83492791};
    uint pint[3] = {0, 0, 0};
    for (uint d = 0; d < 3; ++d)
    {
        pint[d] = static_cast<uint>(std::fabs(p[d])) * primes[d];
    }
    return pint[0] ^ (pint[1] ^ pint[2]);
}

struct PositionHash {
    uint idx = ~0;
    uint hash = ~0;
};

bool PositionHashCmp(const PositionHash& a, const PositionHash& b)
{
    return a.hash < b.hash;
}

void PhysicSystem::Update(const float deltaTime)
{
    ZoneScopedN("PhysicSystem::Update");
    assert(0 <= deltaTime);
    const size_t componentsSize = mComponents.size();
#if 0
    static SpatialAccel::KeyValue* HashTable = SpatialAccel::HashTableInit();
    static std::vector<PositionHash> positionsHashed;
    constexpr float CellLength = 200.f * 0.025f * 2.f;
    {
        ZoneScopedN("Hash and sort");
        positionsHashed.resize(componentsSize);
        for (size_t idx = 0; idx < componentsSize; ++idx)
        {
            PhysicComponent& ci = mComponents[idx];
            if (!ci.IsValid() || !ci.HasFiniteMass())
                continue;
            const glm::vec4& ciPosition = ci.mTransformComponent->mPosition;
            const uint phash = HashPosition3D(ClampPosition3D(glm::vec3(ciPosition), 1.f / CellLength));
            positionsHashed[idx] = {uint(idx), phash};
        }
        std::sort(positionsHashed.begin(), positionsHashed.begin() + componentsSize, PositionHashCmp);

    }

    {
        ZoneScopedN("Update hashtable");
        HashTableClear(HashTable);
        uint currentCell = ~0;
        for (uint idx = 0; idx < componentsSize; ++idx)
        {
            const uint phash = positionsHashed[idx].hash;
            if (phash == currentCell)
                continue;
            currentCell = phash;
            HashTableInsert(HashTable, currentCell, idx);
        }
    }

    for (size_t idx = 0; idx < componentsSize; ++idx)
    {
        PhysicComponent& ci = mComponents[idx];
        if (!ci.IsValid() || !ci.HasFiniteMass())
            continue;
        const float radius = ci.mRadius;
        const float radiusSq = radius * radius;
        const glm::vec4& ciPosition = ci.mTransformComponent->mPosition;
        glm::vec4 ciVelocity = ci.LinearVelocity() * 0.5f;

        const glm::ivec3 CellPosition = ClampPosition3D(glm::vec3(ciPosition), 1.f / CellLength);
        for(int x =-1; x <= 1; ++x)
        for(int y =-1; y <= 1; ++y)
        for(int z =-1; z <= 1; ++z)
        {
            const glm::ivec3 CellOffset = CellPosition + glm::ivec3(x, y, z);
            const uint CellHash = HashPosition3D(CellOffset);
            uint PositionHashedIndex = HashTableLookup(HashTable, CellHash);
            if(SpatialAccel::kEmpty == PositionHashedIndex)
                continue;
            while (true)
            {
                const PositionHash& phash = positionsHashed[PositionHashedIndex];
                if (CellHash != phash.hash)
                    break;
                PositionHashedIndex++;
                if (idx <= phash.idx)
                    continue;
                const PhysicComponent& cy = mComponents[phash.idx];
                if (!cy.IsValid())
                    continue;
                const glm::vec4& cyPosition = cy.mTransformComponent->mPosition;
                const glm::vec4 diffP = ciPosition - cyPosition;
                const float diffMagSq = glm::dot(diffP, diffP);
                if( 0.f == diffMagSq)
                    continue; // superposition
                const float penetrationMagSq = diffMagSq - (4.f * radiusSq);
                if( 0.f < penetrationMagSq)
                    continue; // no penetration
                const float penetrationMag = sqrt(-penetrationMagSq);
                const float diffMag = sqrt(diffMagSq);
                const glm::vec4 diffNormal = diffP / diffMag;
                ciVelocity += diffNormal * penetrationMag;
                if (m_listener)
                {
                    PhysicEvent e = {ci.mEntity, cy.mEntity, &ciVelocity};
                    m_listener->OnPhysicsEvent(e);
                }
            }
        }
        ci.SetLinearVelocity(ciVelocity);
    }

#else
    for (size_t idx = 0; idx < componentsSize; ++idx)
    {
        PhysicComponent& ci = mComponents[idx];
        if (!ci.IsValid() || !ci.HasFiniteMass())
            continue;
        const float radius = ci.mRadius;
        const float radiusSq = radius * radius;
        const glm::vec4& ciPosition = ci.mTransformComponent->mPosition;
        glm::vec4 ciVelocity = ci.LinearVelocity() * 0.5f;
        for (size_t ydx = idx + 1; ydx < componentsSize; ++ydx)
        {
            const PhysicComponent& cy = mComponents[ydx];
            if (!cy.IsValid())
                continue;
            const glm::vec4& cyPosition = cy.mTransformComponent->mPosition;
            const glm::vec4 diffP = ciPosition - cyPosition;
            const float diffMagSq = glm::dot(diffP, diffP);
            if( 0.f == diffMagSq)
                continue; // superposition
            const float penetrationMagSq = diffMagSq - (4.f * radiusSq);
            if( 0.f < penetrationMagSq)
                continue; // no penetration
            const float penetrationMag = sqrt(-penetrationMagSq);
            const float diffMag = sqrt(diffMagSq);
            const glm::vec4 diffNormal = diffP / diffMag;
            ciVelocity += diffNormal * penetrationMag;
            if (m_listener)
            {
                PhysicEvent e = {ci.mEntity, cy.mEntity, &ciVelocity};
                m_listener->OnPhysicsEvent(e);
            }
        }
        ci.SetLinearVelocity(ciVelocity);
    }
#endif
    for (auto& component : mComponents)
    {
        component.Integrate(deltaTime);
    }
}

void PhysicSystem::attachEntity(GameEntity* entity)
{
    PhysicComponent& component = IComponentSystem::attachComponent<PhysicComponent>(entity, mComponents);
    TransformComponent* tranform = entity->getComponent<TransformComponent>();
    assert(tranform);
    component.mTransformComponent = tranform;
    component.mEntity = entity;
}

void PhysicSystem::detachEntity(GameEntity* entity)
{
    IComponentSystem::detachComponent<PhysicComponent>(entity, mComponents);
}
