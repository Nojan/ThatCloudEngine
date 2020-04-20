#include "select_system.hpp"

#include "boundingbox.hpp"
#include "rendering_system.hpp"
#include "visualdebug.hpp"

struct Ray {
    glm::vec3 mStart;
    glm::vec3 mDir;
    glm::vec3 mDirInv;

    static Ray make_ray(const glm::vec3& start, const glm::vec3& end);
    static bool intersect(const Ray& r, const BoundingBox3D& bbox, float& tmin, float& tmax);
};

Ray Ray::make_ray(const glm::vec3& start, const glm::vec3& end)
{
    Ray r;
    r.mStart = start;
    r.mDir = glm::normalize(end - start);
    r.mDirInv = 1.f / r.mDir;
    return r;
}

bool Ray::intersect(const Ray& r, const BoundingBox3D& bbox, float& tmin, float& tmax)
{
    for (int idx = 0; idx < 3; ++idx)
    {
        const float p = r.mStart[idx];
        const float d = r.mDir[idx];
        const float bmin = bbox.Min()[idx];
        const float bmax= bbox.Max()[idx];
        if (fabsf(d) < 0.0001f)
        {
            if(p < bmin || p > bmax)
                return false;
        }
        else
        {
            const float dInv = 1.0f / d;
            float t1 = (bmin - p) * dInv;
            float t2 = (bmax - p) * dInv;

            if(t1 > t2) std::swap(t1, t2);

            tmin = glm::max(tmin, t1);
            tmax = glm::min(tmax, t2);

            if(tmin > tmax)
                return false;
        }
    }
    return true;
}

SelectComponent::SelectComponent()
{
}

SelectComponent::SelectComponent(const SelectComponent& ref)
: mEntity(ref.mEntity)
{
}

bool SelectComponent::Invalid() const
{
    return nullptr == mEntity;
}

SelectSystem::SelectSystem()
{
    mComponents.reserve(GameEntity::Max);
}

SelectSystem::~SelectSystem()
{
}

void SelectSystem::SelectWithRay(const glm::vec3& start, const glm::vec3& end)
{
    Unselect();
    const Ray r = Ray::make_ray(start, end);
    float tmin = FLT_MAX;
    for(size_t idx = 0; idx < mComponents.size(); ++idx)
    {
        GameEntity* selectedEntity = mComponents[idx].mEntity;
        const BoundingBox3D selectedBBox = selectedEntity->getComponent<GraphicMeshComponent>()->getBoundingBox();
        float idx_tmin = 0;
        float idx_tmax = tmin;
        if( Ray::intersect(r, selectedBBox, idx_tmin, idx_tmax) )
        {
            tmin = idx_tmin;
            mSelected = idx;
        }
    }
}

void SelectSystem::Unselect()
{
    mSelected = -1;
}

GameEntity* SelectSystem::GetSelected()
{
    GameEntity* result = nullptr;
    if (-1 != mSelected)
    {
        result = mComponents[mSelected].mEntity;
    }
    return result;
}

void SelectSystem::FrameStep()
{
    if (-1 == mSelected)
        return;

    GameEntity* selectedEntity = mComponents[mSelected].mEntity;
    BoundingBox3D selectedBBox = selectedEntity->getComponent<GraphicMeshComponent>()->getBoundingBox();
    const glm::vec3 center = selectedBBox.Center();
    const glm::vec3 extent = selectedBBox.Extent() * 0.55f;
    BoundingBox3D scaledBBox(center - extent, center + extent);
    VisualDebugBoundingBoxCommand command(scaledBBox, { 1, 0, 0, 1 }, glm::mat4(1.0f), true);
    VisualDebug()->PushCommand(command);
}

void SelectSystem::attachEntity(GameEntity* entity)
{
    SelectComponent& component = IComponentSystem::attachComponent<SelectComponent>(entity, mComponents);
    component.mEntity = entity;
}

void SelectSystem::detachEntity(GameEntity* entity)
{
    IComponentSystem::detachComponent<SelectComponent>(entity, mComponents);
}
