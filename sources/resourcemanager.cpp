#include "resourcemanager.hpp"

#include "mesh_resource_cache.hpp"
#include "resourcecache.hpp"
#include "skinMeshCache.hpp"
#include "shadercache.hpp"
#include "sound_stream_cache.hpp"
#include "texture_cache.hpp"

ResourceManager::ResourceManager()
: mMeshResourceCache(std::make_unique<MeshResourceCache>())
, mTextureCache(std::make_unique<Texture2DCache>())
, mResourceCache(std::make_unique<ResourceCache>())
{
    mSkinMeshCache.reset(new SkinMeshCache());
    mShaderCache.reset(new ShaderCache());
    mSoundStreamCache.reset(new SoundStreamCache());
}

ResourceManager::~ResourceManager()
{
}

ResourceCache *ResourceManager::Cache()
{
    return mResourceCache.get();
}

std::shared_ptr<MeshResourceList> ResourceManager::meshResource(const std::string & resourceName)
{
    return mMeshResourceCache->get(resourceName);
}

std::shared_ptr<SkinMesh> ResourceManager::skinMesh(const std::string & resourceName)
{
    return mSkinMeshCache->get(resourceName);
}

std::shared_ptr<ShaderProgram> ResourceManager::shader(const std::string& resourceName)
{
    return mShaderCache->get(resourceName);
}

std::shared_ptr<SoundStream> ResourceManager::soundStream(const std::string& resourceName)
{
    return mSoundStreamCache->get(resourceName);
}

std::shared_ptr<Texture2D> ResourceManager::texture(const std::string & resourceName)
{
    return mTextureCache->get(resourceName);
}
