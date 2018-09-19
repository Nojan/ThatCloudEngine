#include "resource_compiler_mesh.hpp"

#include "renderableMesh.hpp"
#include "global.hpp"
#include "platform/platform.hpp"
#include "resource_compiler.hpp"

#include "tinyxml/tinyxml2.h"

namespace resource_compiler {

void compile_mesh(const char* filepath, Mesh& mesh) {
    Platform* platform = Global::platform();
    FileHandle fileHandle = platform->OpenFile(filepath, "rb");
    FILE* file = fileHandle.get();
    assert(file);
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError error = doc.LoadFile(file);
    assert(!error);
    const tinyxml2::XMLElement* visualSceneElement = doc.FirstChildElement("ASSIMP")->FirstChildElement("Scene");
    const tinyxml2::XMLElement* materialList = visualSceneElement->FirstChildElement("MaterialList");
    assert(1 == materialList->IntAttribute("num")); // as long there is only one material, we can merge meshes
    const tinyxml2::XMLElement* meshList = visualSceneElement->FirstChildElement("MeshList");
    for (const tinyxml2::XMLElement* meshElement = meshList->FirstChildElement("Mesh"); meshElement != nullptr; meshElement = meshElement->NextSiblingElement("Mesh"))
    {
        const uint offset =  mesh.mVertex.size();
        GetVertex(meshElement, mesh.mVertex);
        GetNormal(meshElement, mesh.mNormal);
        GetUV(meshElement, mesh.mTextureCoord);
        GetFace(meshElement, mesh.mIndex, offset);

        const tinyxml2::XMLElement* positionsElement = meshElement->FirstChildElement("Positions");
        const int vertexCount = positionsElement->IntAttribute("num") + offset;
        assert(vertexCount == mesh.mVertex.size());
        assert(vertexCount == mesh.mNormal.size());
        assert(vertexCount == mesh.mTextureCoord.size());
    }

    const uint vertexCount = mesh.mVertex.size();
    for (uint faceIdx = 0; faceIdx < mesh.mIndex.size(); ++faceIdx)
    {
        assert(mesh.mIndex[faceIdx] < vertexCount);
    }
}

} //resource_compiler
