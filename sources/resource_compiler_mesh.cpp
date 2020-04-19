#include "resource_compiler_mesh.hpp"

#include "renderableMesh.hpp"
#include "global.hpp"
#include "resourcefile.hpp"
#include "resourcecache.hpp"
#include "resourcemanager.hpp"
#include "platform/platform.hpp"
#include "resource_compiler.hpp"

#include "tinyxml/tinyxml2.h"

namespace resource_compiler {

void compile_mesh(const char * filepath, MeshResourceList& meshList)
{
    Platform* platform = Global::platform();
    FileHandle fileHandle = platform->OpenFile(filepath, "rb");
    FILE* file = fileHandle.get();
    assert(file);
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError error = doc.LoadFile(file);
    assert(!error);
    const tinyxml2::XMLElement* visualSceneElement = doc.FirstChildElement("ASSIMP")->FirstChildElement("Scene");
    const tinyxml2::XMLElement* materialList = visualSceneElement->FirstChildElement("MaterialList");
    const tinyxml2::XMLElement* meshListElement = visualSceneElement->FirstChildElement("MeshList");
    for (const tinyxml2::XMLElement* meshElement = meshListElement->FirstChildElement("Mesh"); meshElement != nullptr; meshElement = meshElement->NextSiblingElement("Mesh"))
    {
        MeshResource meshResource;
        
        int materialId = -1;
        if(tinyxml2::XML_NO_ERROR == meshElement->QueryIntAttribute("material_index", &materialId))
        {
            int materialIdx = 0;
            for (const tinyxml2::XMLElement* materialElement = materialList->FirstChildElement("Material"); materialElement != nullptr; materialElement = materialElement->NextSiblingElement("Material"))
            {
                if (materialId == materialIdx)
                {
                    if (const tinyxml2::XMLElement* materialPropListElement = materialElement->FirstChildElement("MatPropertyList"))
                    {
                        for (const tinyxml2::XMLElement* materialPropElement = materialPropListElement->FirstChildElement("MatProperty"); materialPropElement != nullptr; materialPropElement = materialPropElement->NextSiblingElement("MatProperty"))
                        {
                            if (materialPropElement->Attribute("key", "$tex.file"))
                            {
                                const char* textureNameDirty = materialPropElement->GetText();
                                // TODO fix data
                                char textureName[64];
                                memset(textureName, '\0', 64);
                                for (int idx = 0; idx < 64 && *textureNameDirty != '\0'; ++textureNameDirty)
                                {
                                    if (isalnum(*textureNameDirty) || '.' == *textureNameDirty || '_' == *textureNameDirty)
                                    {
                                        textureName[idx] = *textureNameDirty;
                                        ++idx;
                                    }
                                }
                                char folder[256];
                                memset(folder, '\0', sizeof(folder));
                                if (const char* lastForwardSlash = strrchr(filepath, '/'))
                                {
                                    const size_t length = lastForwardSlash - filepath + 1;
                                    assert(length < sizeof(folder));
                                    strncpy(folder, filepath, length);
                                }
                                char filename[256];
                                sprintf(filename, "%s%s", folder, textureName);
                                meshResource.m_texture = Global::resourceManager()->texture(filename);
                                break;
                            }
                        }
                    }
                    break;
                }
                ++materialIdx;
            }
        }
        else
        {
            meshResource.m_texture = Global::resourceManager()->texture("default");
        }

        meshResource.m_mesh = std::make_shared<Mesh>();
        Mesh& mesh = *(meshResource.m_mesh);
        const uint offset =  mesh.mVertex.size();
        GetVertex(meshElement, mesh.mVertex);
        GetNormal(meshElement, mesh.mNormal);
        GetUV(meshElement, mesh.mTextureCoord);
        GetFace(meshElement, mesh.mIndex, offset);

        const tinyxml2::XMLElement* positionsElement = meshElement->FirstChildElement("Positions");
        const uint vertexCount = positionsElement->IntAttribute("num") + offset;
        assert(vertexCount == mesh.mVertex.size());
        assert(vertexCount == mesh.mNormal.size());
        assert(vertexCount == mesh.mTextureCoord.size());

        for (uint faceIdx = 0; faceIdx < mesh.mIndex.size(); ++faceIdx)
        {
            assert(mesh.mIndex[faceIdx] < vertexCount);
        }

        for (uint idx = offset; idx < vertexCount; ++idx)
        {
            mesh.mBBox.Add(mesh.mVertex[idx]);
        }
        
        meshList.push_back(meshResource);
    }
}

void get_dependencies(const ResourceFile& meshfile, std::vector<std::shared_ptr<ResourceFile>>& dependencies)
{
    Platform* platform = Global::platform();
    FileHandle fileHandle = platform->OpenFile(meshfile.name().c_str(), "rb");
    FILE* file = fileHandle.get();
    assert(file);
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError error = doc.LoadFile(file);
    assert(!error);
    const tinyxml2::XMLElement* visualSceneElement = doc.FirstChildElement("ASSIMP")->FirstChildElement("Scene");
    const tinyxml2::XMLElement* materialList = visualSceneElement->FirstChildElement("MaterialList");
    const tinyxml2::XMLElement* meshListElement = visualSceneElement->FirstChildElement("MeshList");
    for (const tinyxml2::XMLElement* meshElement = meshListElement->FirstChildElement("Mesh"); meshElement != nullptr; meshElement = meshElement->NextSiblingElement("Mesh"))
    {
        int materialId = -1;
        if (tinyxml2::XML_NO_ERROR == meshElement->QueryIntAttribute("material_index", &materialId))
        {
            int materialIdx = 0;
            for (const tinyxml2::XMLElement* materialElement = materialList->FirstChildElement("Material"); materialElement != nullptr; materialElement = materialElement->NextSiblingElement("Material"))
            {
                if (materialId == materialIdx)
                {
                    if (const tinyxml2::XMLElement* materialPropListElement = materialElement->FirstChildElement("MatPropertyList"))
                    {
                        for (const tinyxml2::XMLElement* materialPropElement = materialPropListElement->FirstChildElement("MatProperty"); materialPropElement != nullptr; materialPropElement = materialPropElement->NextSiblingElement("MatProperty"))
                        {
                            if (materialPropElement->Attribute("key", "$tex.file"))
                            {
                                const char* textureNameDirty = materialPropElement->GetText();
                                // TODO fix data
                                char textureName[64];
                                memset(textureName, '\0', 64);
                                for (int idx = 0; idx < 64 && *textureNameDirty != '\0'; ++textureNameDirty)
                                {
                                    if (isalnum(*textureNameDirty) || '.' == *textureNameDirty || '_' == *textureNameDirty)
                                    {
                                        textureName[idx] = *textureNameDirty;
                                        ++idx;
                                    }
                                }
                                char folder[256];
                                memset(folder, '\0', sizeof(folder));
                                const char* filepath = meshfile.name().c_str();
                                if (const char* lastForwardSlash = strrchr(filepath, '/'))
                                {
                                    const size_t length = lastForwardSlash - filepath + 1;
                                    assert(length < sizeof(folder));
                                    strncpy(folder, filepath, length);
                                }
                                char filename[256];
                                sprintf(filename, "%s%s", folder, textureName);
                                dependencies.push_back(Global::resourceManager()->Cache()->get_or_create<ResourceFile>(filename));
                                break;
                            }
                        }
                    }
                    break;
                }
                ++materialIdx;
            }
        }
    }
}

} //resource_compiler
