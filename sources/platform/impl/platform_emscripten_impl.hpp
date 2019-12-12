#pragma once
#include "iplatformimpl.hpp"
#include <atomic>

class PlatformEmscripten : public IPlatformImpl {
public:
    PlatformEmscripten();

    void Init() override;
    bool Ready() const override;
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
};
