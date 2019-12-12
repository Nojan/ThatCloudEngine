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
    const char * url[] = {
        "../assets/Sounds/ingame.ogg",
        "../assets/3D/cloudboy_slow.assxml",
        "../assets/3D/cloudboyskin.tga",
        "../assets/3D/cloudboycloth.tga",
        "../assets/3D/cloudboy_normal.assxml",
        "../assets/3D/cloudboy_fast.assxml",
        "../assets/3D/CursorBillboard_1_0.tga",
        "../assets/3D/CursorBillboard_1_1.tga",
        "../assets/3D/CursorBillboard_1_2.tga",
        "../assets/3D/CursorBillboard_1_3.tga",
        "../assets/3D/CursorBillboard_1_4.tga",
        "../assets/3D/CursorBillboard_1_5.tga",
        "../assets/3D/CursorBillboard_1_6.tga",
        "../assets/3D/CursorBillboard_1_7.tga",
        "../assets/3D/CursorBillboard_1_8.tga",
        "../assets/3D/CursorBillboard_1_9.tga",
        "../assets/3D/CursorBillboard_1_10.tga",
        "../assets/3D/CursorBillboard_1_11.tga",
        "../assets/3D/volcano.tga",
        "../assets/3D/volcano_beach.tga",
        "../assets/3D/city.tga",
        "../assets/3D/paw.tga",
        "../assets/3D/albino.tga",
        "../assets/3D/wet.tga",
        "../assets/3D/big.tga",
        "../assets/3D/pacman.tga",
        "../assets/3D/good.tga",
        "../assets/3D/oceanbottom.tga",
        "../assets/3D/shallowwater.tga",
        "../assets/3D/beach.tga",
        "../assets/3D/ocean.tga",
        "../assets/3D/tree.tga",
    };
    const size_t count = (sizeof(url) / sizeof(url[0]));
    for (size_t idx = 0; idx < count; ++idx) {
        Fetch(url[idx]);
    }
}

bool PlatformEmscripten::Ready()
{
#ifndef __EMSCRIPTEN__
    for (size_t idx = mFileLoading.size() - 1; idx < mFileLoading.size(); --idx)
    {
        FileFetch& file = mFileLoading[idx];
        file.delay--;
        if (file.delay <= 0)
        {
            OnLoad(file.name.c_str());
            mFileLoading[idx] = mFileLoading[mFileLoading.size() - 1];
            mFileLoading.resize(mFileLoading.size() - 1);
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
            mFileLoading.push_back({std::string(filename), int(size)});
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
    // Resource are not necessary in the cache.
    // TODO use a callback to ResourceFile.
    ResourceCache* resourceCache = Global::resourceManager()->Cache();
    if (std::shared_ptr<ResourceFile> file = resourceCache->get<ResourceFile>(filename))
    {
        file->SetLoadingSuccess(success);
    }
}
