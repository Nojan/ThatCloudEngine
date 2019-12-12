#pragma once

#include "iplatformimpl.hpp"

class PlatformWindows : public IPlatformImpl {
public:
    void Init() override;
    bool Ready() override;
    void Terminate() override;

    bool Fetch(const char* filename) override;
    FILE * OpenFile(const char* filename, const char * mode) override;
    void CloseFile(FILE * file) override;

private:
    FILE * mLogFile = nullptr; 
};
