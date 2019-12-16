#include "platform_emscripten_impl.hpp"
#include "../../global.hpp"
#include "../../resourcecache.hpp"
#include "../../resourcemanager.hpp"
#include "../../resourcefile.hpp"

#include <cassert>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

static PlatformEmscripten* gloPlatformEmscripten = nullptr;

PlatformEmscripten::PlatformEmscripten()
: mFileToLoad(0)
{
    assert(nullptr == gloPlatformEmscripten);
    gloPlatformEmscripten = this;
}

void PlatformEmscripten::Init()
{
    printf("Emscripten FS init\n");
    printf("Create dir ../assets\n");
#ifdef __EMSCRIPTEN__
    EM_ASM(
        FS.mkdir('/../assets');
        FS.mount(MEMFS, {}, '/../assets');
        FS.mkdir('/../shaders');
        FS.mount(MEMFS, {}, '/../shaders');
    );
#endif
    printf("Emscripten FS init done\n");
}

bool PlatformEmscripten::Ready()
{
#ifndef __EMSCRIPTEN__
    if (!mFileLoading.empty())
    {
        mFileLoading[0].delay -= 64;
        for (size_t idx = mFileLoading.size() - 1; idx < mFileLoading.size(); --idx)
        {
            FileFetch& file = mFileLoading[idx];
            if (file.delay <= 0)
            {
                OnLoad(file.name.c_str());
                mFileLoading[idx] = mFileLoading[mFileLoading.size() - 1];
                mFileLoading.resize(mFileLoading.size() - 1);
            }
        }
    }
#endif
    return 0 == mFileToLoad;
}

void PlatformEmscripten::Terminate() {

}

bool PlatformEmscripten::Fetch(const char *filename)
{
        ++mFileToLoad;
        const char * url = filename;
        printf("async_wget(%s, %s)\n", url, filename);
#ifdef __EMSCRIPTEN__
        auto onLoadFunc = [](const char* filename) { gloPlatformEmscripten->OnLoad(filename); };
        auto onErrorFunc = [](const char* filename) { gloPlatformEmscripten->OnLoadError(filename); };
        emscripten_async_wget(url, filename, onLoadFunc, onErrorFunc);
#else
        if(FILE* file = fopen(filename, "rb"))
        {
            fseek(file, 0, SEEK_END);
            size_t size = ftell(file);
            fseek(file, 0, SEEK_SET);
            fclose(file);
            mFileLoading.push_back({std::string(filename), int(size / 1024)});
        }
        else
        {
            OnLoadError(filename);
        }
#endif
        return false;
}

FILE * PlatformEmscripten::OpenFile(const char* filename, const char * mode) {
    return fopen(filename, mode);
}

void PlatformEmscripten::CloseFile(FILE * file) {
    fclose(file);
}

void PlatformEmscripten::OnLoad(const char * filename)
{
    printf("wget success %s\n", filename);
    OnLoadSuccess(filename, true);
    --mFileToLoad;
}

void PlatformEmscripten::OnLoadError(const char * filename)
{
    printf("wget error %s\n", filename);
    OnLoadSuccess(filename, false);
}

void PlatformEmscripten::OnLoadSuccess(const char* filename, bool success)
{
    assert(filename);
    // Resource are not necessary in the cache.
    // TODO use a callback to ResourceFile.
    std::string filenameInCache;
    if (filename == strstr(filename, ".."))
    {
        filenameInCache = filename;
    } else {
        // weird emscripten behaviour: requesting ../assets/file will succeed with filename /assets/file. 
        filenameInCache = std::string("..") + std::string(filename);
    }
    ResourceCache* resourceCache = Global::resourceManager()->Cache();
    if (std::shared_ptr<ResourceFile> file = resourceCache->get<ResourceFile>(filenameInCache))
    {
        file->SetLoadingSuccess(success);
    }
}
