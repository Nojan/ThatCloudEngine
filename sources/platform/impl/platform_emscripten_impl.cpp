#include "platform_emscripten_impl.hpp"

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
    printf("Create dir ../asset\n");
#ifdef __EMSCRIPTEN__
    EM_ASM(
        FS.mkdir('/../asset');
    FS.mount(MEMFS, {}, '/../asset');
    FS.mkdir('/../asset/mesh');
    FS.mount(MEMFS, {}, '/../asset/mesh');
    FS.mkdir('/../asset/sound');
    FS.mount(MEMFS, {}, '/../asset/sound');
    FS.mkdir('/../asset/texture');
    FS.mount(MEMFS, {}, '/../asset/texture');
    );
#endif
    printf("Emscripten FS init done\n");
    const char * url[] = {
        "../shaders/skybox.vert",
        "../shaders/skybox.frag",
        "../shaders/texture.vert",
        "../shaders/texture.frag",
        "../shaders/visualdebug.vert",
        "../shaders/visualdebug.frag",
        "../shaders/skin.vert",
        "../shaders/skin.frag",
        "../shaders/billboard.vert",
        "../shaders/billboard.frag",
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
        "../assets/3D/cloud_1_1.tga",
        "../assets/3D/cloud_1_2.tga",
        "../assets/3D/cloud_1_3.tga",
        "../assets/3D/cloud_1_4.tga",
        "../assets/3D/cloud_1_5.tga",
        "../assets/3D/cloud_1_6.tga",
        "../assets/3D/cloud_1_7.tga",
        "../assets/3D/wave_5_0.tga",
        "../assets/3D/wave_5_1.tga",
        "../assets/3D/wave_5_2.tga",
        "../assets/3D/wave_5_3.tga",
        "../assets/3D/wave_5_4.tga",
        "../assets/3D/wave_5_5.tga",
        "../assets/3D/wave_5_6.tga",
        "../assets/3D/wave_5_7.tga",
        "../assets/3D/wave_5_8.tga",
        "../assets/3D/wave_5_9.tga",
        "../assets/3D/wave_5_10.tga",
        "../assets/3D/wave_5_11.tga",
        "../assets/3D/wave_5_12.tga",
        "../assets/3D/wave_5_13.tga",
        "../assets/3D/wave_5_14.tga",
        "../assets/3D/wave_5_15.tga",
        "../assets/3D/wave_5_16.tga",
        "../assets/3D/wave_5_17.tga",
        "../assets/3D/wave_5_18.tga",
        "../assets/3D/wave_5_19.tga",
        "../assets/3D/wave_5_20.tga",
        "../assets/3D/wave_5_21.tga",
        "../assets/3D/wave_5_22.tga",
        "../assets/3D/wave_5_23.tga",
        "../assets/3D/wave_5_24.tga",
        "../assets/3D/wave_5_25.tga",
        "../assets/3D/wave_5_26.tga",
        "../assets/3D/wave_5_27.tga",
        "../assets/3D/wave_5_28.tga",
        "../assets/3D/wave_5_29.tga",
        "../assets/3D/wave_5_30.tga",
        "../assets/3D/wave_5_31.tga",
        "../assets/3D/wave_5_32.tga",
        "../assets/3D/wave_5_33.tga",
        "../assets/3D/wave_5_34.tga",
        "../assets/3D/wave_5_35.tga",
        "../assets/3D/wave_5_36.tga",
        "../assets/3D/wave_5_37.tga",
        "../assets/3D/wave_5_38.tga",
        "../assets/3D/wave_5_39.tga",
        "../assets/3D/wave_5_40.tga",
        "../assets/3D/wave_5_41.tga",
        "../assets/3D/wave_5_42.tga",
        "../assets/3D/wave_5_43.tga",
        "../assets/3D/wave_5_44.tga",
        "../assets/3D/wave_5_45.tga",
        "../assets/3D/wave_5_46.tga",
        "../assets/3D/wave_5_47.tga",
        "../assets/3D/wave_5_48.tga",
        "../assets/3D/wave_5_49.tga",
        "../assets/3D/wave_5_50.tga",
        "../assets/3D/wave_5_51.tga",
        "../assets/3D/wave_5_52.tga",
        "../assets/3D/wave_5_53.tga",
        "../assets/3D/wave_5_54.tga",
        "../assets/3D/wave_5_55.tga",
        "../assets/3D/wave_5_56.tga",
        "../assets/3D/wave_5_57.tga",
        "../assets/3D/wave_5_58.tga",
        "../assets/3D/wave_5_59.tga",
        "../assets/3D/islandvolcano.assxml",
        "../assets/3D/volcano.tga",
        "../assets/3D/volcano_beach.tga",
        "../assets/3D/cityvolcano.assxml",
        "../assets/3D/city.tga",
        "../assets/3D/islandsrest.assxml",
        "../assets/3D/paw.tga",
        "../assets/3D/albino.tga",
        "../assets/3D/wet.tga",
        "../assets/3D/big.tga",
        "../assets/3D/pacman.tga",
        "../assets/3D/island3big.assxml",
        "../assets/3D/good.tga",
        "../assets/3D/oceanbottom_7.assxml",
        "../assets/3D/oceanbottom.tga",
        "../assets/3D/shallowwater5volcano.assxml",
        "../assets/3D/shallowwater.tga",
        "../assets/3D/shallowwater5rest.assxml",
        "../assets/3D/shallowwater4rest.assxml",
        "../assets/3D/beach.tga",
        "../assets/3D/shallowwater43big.assxml",
        "../assets/3D/ocean_3.assxml",
        "../assets/3D/ocean.tga",
        "../assets/3D/beachvolcano.assxml",
        "../assets/3D/beachrest.assxml",
        "../assets/3D/beach3big.assxml",
        "../assets/3D/wavevolcano.assxml",
        "../assets/3D/wave3big.assxml",
        "../assets/3D/waverest.assxml",
        "../assets/3D/treevolcano.assxml",
        "../assets/3D/tree.tga",
        "../assets/3D/treerest.assxml",
        "../assets/3D/tree3big.assxml",
        "../assets/Cloud/Levels/Yun.xml",
        "../assets/Sounds/cloud_release.ogg",
        "../assets/Sounds/cloud_consume.ogg",
        "../assets/Sounds/cloud_normaltopurified.ogg",
    };
    const size_t count = (sizeof(url) / sizeof(url[0]));
    mFileToLoad = count;

    auto onLoadFunc = [](const char* filename) { gloPlatformEmscripten->OnLoad(filename); };
    auto onErrorFunc = [](const char* filename) { gloPlatformEmscripten->OnLoadError(filename); };

    for (size_t idx = 0; idx < count; ++idx) {
        const char * filename = url[idx];
        const char * url = filename;
        printf("async_wget(%s, %s)\n", url, filename);
#ifdef __EMSCRIPTEN__
        emscripten_async_wget(url, filename, onLoadFunc, onErrorFunc);
#endif
    };
}

bool PlatformEmscripten::Ready() const
{
    return 0 == mFileToLoad;
}

void PlatformEmscripten::Terminate() {

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
    --mFileToLoad;
}

void PlatformEmscripten::OnLoadError(const char * filename)
{
    printf("wget error %s\n", filename);
}
