#pragma once
#include "iplatformimpl.hpp"
#include <atomic>
#ifndef __EMSCRIPTEN__
#include <string>
#include <vector>
#endif

class PlatformEmscripten : public IPlatformImpl {
public:
    PlatformEmscripten();

    void Init() override;
    bool Ready() override;
    void Terminate() override;

    bool Fetch(const char* filename) override;
    FILE * OpenFile(const char* filename, const char * mode) override;
    void CloseFile(FILE * file) override;

public:
    void OnLoad(const char * filename);
    void OnLoadError(const char * filename);
    void OnLoadSuccess(const char* filename, bool success);

private:
    std::atomic_uint mFileToLoad;
#ifndef __EMSCRIPTEN__
    struct FileFetch
    {
        std::string name;
        int delay;
    };
    std::vector<FileFetch> mFileLoading;
#endif
};
