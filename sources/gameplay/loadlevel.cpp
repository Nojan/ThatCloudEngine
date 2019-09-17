#include "loadlevel.hpp"

#include "gridsystem.hpp"
#include "gameconstant.hpp"

#include "../types.hpp"
#include "../global.hpp"
#include "../platform/platform.hpp"
#include "../game_entity.hpp"
#include "../game_system.hpp"
#include "../physic_system.hpp"
#include "../root.hpp"
#include "../resourcemanager.hpp"
#include "../rendering_system.hpp"
#include "../renderableMesh.hpp"
#include "../transform_system.hpp"
#include "../tinyxml/tinyxml2.h"

void loadlevel(const char * filepath, CloudSpawner cloudSpawner, GridSpawner gridSpawner, glm::vec3& boyPosition, glm::vec3& cameraOffset)
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
    const float defaultAltitude = Gameplay::grid_altitude;
    GameSystem* gameSystem = Global::gameSytem();
    int idx = 0;
    for (const tinyxml2::XMLElement* cloudElement = CloudsElement->FirstChildElement("cloud"); cloudElement != nullptr; cloudElement = cloudElement->NextSiblingElement("cloud"))
    {
        const float x = cloudElement->FloatAttribute("x") * scale;
        const float z = -cloudElement->FloatAttribute("y") * scale;
        const int color = cloudElement->IntAttribute("color");
        const float power = cloudElement->FloatAttribute("power");
        const glm::vec3 position(x, defaultAltitude, z);
        cloudSpawner(position, color, power);
    }

    if (const tinyxml2::XMLElement* gridElement = CloudLevelElement->FirstChildElement("Grid")) {
        for (const tinyxml2::XMLElement* gridsetElement = gridElement->FirstChildElement("GridSet"); gridsetElement != nullptr; gridsetElement = gridsetElement->NextSiblingElement("GridSet"))
        {
            const char* gridsetName = gridsetElement->Attribute("name");
            for (const tinyxml2::XMLElement* gridcellElement = gridsetElement->FirstChildElement("GridCell"); gridcellElement != nullptr; gridcellElement = gridcellElement->NextSiblingElement("GridCell"))
            {
                const glm::ivec2 gridCellPosition = GridCellSystem::GetWorldPosition( glm::ivec2(gridcellElement->IntAttribute("x"), gridcellElement->IntAttribute("y")) );
                gridSpawner(gridsetName, gridCellPosition.x, gridCellPosition.y);
            }
        }
    }
}
