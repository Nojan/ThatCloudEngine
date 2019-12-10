#pragma once

#include "mesh_resource.hpp"
#include <string>
#include <memory>

struct MeshResource;
class MeshResourceCache;
struct SkinMesh;
class SkinMeshCache;
class ShaderCache;
class ShaderProgram;
struct SoundStream;
class SoundStreamCache;
class Texture2D;
class Texture2DCache;
class ResourceCache;

class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();

    ResourceCache* Cache();
    
    std::shared_ptr<MeshResourceList> meshResource(const std::string& resourceName);
    std::shared_ptr<SkinMesh> skinMesh(const std::string& resourceName);
    std::shared_ptr<ShaderProgram> shader(const std::string& resourceName);
    std::shared_ptr<SoundStream> soundStream(const std::string& resourceName);
    std::shared_ptr<Texture2D> texture(const std::string& resourceName);

private:
    std::unique_ptr<MeshResourceCache> mMeshResourceCache;
    std::unique_ptr<SkinMeshCache> mSkinMeshCache;
    std::unique_ptr<ShaderCache> mShaderCache;
    std::unique_ptr<SoundStreamCache> mSoundStreamCache;
    std::unique_ptr<Texture2DCache> mTextureCache;
    std::unique_ptr<ResourceCache> mResourceCache;
};

