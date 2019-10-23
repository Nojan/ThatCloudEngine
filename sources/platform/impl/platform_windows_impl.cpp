#include "platform_windows_impl.hpp"

void PlatformWindows::Init() {
    if (!mLogFile)
    {
        mLogFile = fopen("opened_res.txt", "w");
    }
}

bool PlatformWindows::Ready() const
{
    return true;
}

void PlatformWindows::Terminate() {
    if (mLogFile)
    {
        fclose(mLogFile);
        mLogFile = nullptr;
    }
}

FILE * PlatformWindows::OpenFile(const char* filename, const char * mode) {
    if (mLogFile)
    {
        fprintf(mLogFile, "\"%s\",\n", filename);
    }
    return fopen(filename, mode);
}

void PlatformWindows::CloseFile(FILE * file) {
    fclose(file);
}
