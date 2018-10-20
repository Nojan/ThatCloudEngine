#include "loadlevel.hpp"

#include "../types.hpp"
#include "../billboard_rendering_system.hpp"
#include "../global.hpp"
#include "../platform/platform.hpp"
#include "../game_entity.hpp"
#include "../game_system.hpp"
#include "../root.hpp"
#include "../resourcemanager.hpp"
#include "../rendering_system.hpp"
#include "../renderableMesh.hpp"
#include "../texture.hpp"
#include "../transform_system.hpp"
#include "../tinyxml/tinyxml2.h"

void loadlevel(const char * filepath, std::vector<GameEntity*>& entities, glm::vec3& boyPosition, glm::vec3& cameraOffset)
{
    std::vector<std::shared_ptr<Texture2D>> cloudsTextures;
    cloudsTextures.reserve(7);
    for (int idx = 1; idx <= 7; ++idx)
    {
        char filename[256];
        sprintf(filename, "../assets/3D/cloud_1_%d.tga", idx);

        cloudsTextures.push_back( Global::resourceManager()->texture(filename) );
    }
    
    Platform* platform = Global::platform();
    FileHandle fileHandle = platform->OpenFile(filepath, "rb");
    FILE* file = fileHandle.get();
    assert(file);
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError error = doc.LoadFile(file);
    assert(!error);

    const tinyxml2::XMLElement* CloudLevelElement = doc.FirstChildElement("CloudLevel");
    if (const tinyxml2::XMLElement* BoyElement = CloudLevelElement->FirstChildElement("Boy"))
    {
        const float x = BoyElement->FloatAttribute("x");
        const float y = BoyElement->FloatAttribute("y");
        const float z = BoyElement->FloatAttribute("z");
        boyPosition = glm::vec3(x, y, z);
    }

    if (const tinyxml2::XMLElement* CameraElement = CloudLevelElement->FirstChildElement("CameraStart"))
    {
        const float x = CameraElement->FloatAttribute("Distance");
        const float y = CameraElement->FloatAttribute("HRot");
        const float z = CameraElement->FloatAttribute("VRot");
        cameraOffset = glm::vec3(x, y, z);
    }

    const tinyxml2::XMLElement* CloudsElement = CloudLevelElement->FirstChildElement("CloudLayer")->FirstChildElement("Clouds");
    const float scale = 200.f;
    const float defaultAltitude = 160.f;
    GameSystem* gameSystem = Global::gameSytem();
    int idx = 0;
    for (const tinyxml2::XMLElement* cloudElement = CloudsElement->FirstChildElement("cloud"); cloudElement != nullptr; cloudElement = cloudElement->NextSiblingElement("cloud"))
    {
        const float x = cloudElement->FloatAttribute("x") * scale;
        const float z = -cloudElement->FloatAttribute("y") * scale;
        const glm::vec4 position(x, defaultAltitude, z, 1.f);

        GameEntity* entity = gameSystem->createEntity();
        entities.push_back(entity);
        gameSystem->getSystem<TransformSystem>()->attachEntity(entity);
        TransformComponent* transform = entity->getComponent<TransformComponent>();
        transform->SetPosition(position);

        gameSystem->getSystem<BillboardRenderingSystem>()->attachEntity(entity);
        BillboardComponent* billboardComponent = entity->getComponent<BillboardComponent>();
        billboardComponent->mColor = { 0.f, 0.f, 1.f, 1.f };
        billboardComponent->mBillboard.mTexture = cloudsTextures[++idx % 7];
        billboardComponent->mBillboard.mSize = glm::vec2(25.f);
        billboardComponent->mBillboard.mAlpha = 1.f;
    }
}
