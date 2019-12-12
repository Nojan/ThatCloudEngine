#include "platform_windows_impl.hpp"

void PlatformWindows::Init() {
    if (!mLogFile)
    {
        mLogFile = fopen("opened_res.txt", "w");
    }
}

bool PlatformWindows::Ready()
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

bool PlatformWindows::Fetch(const char *filename)
{
    bool res = false;
    if(FILE* f = fopen(filename, "rb"))
    {
       res = true;
       fclose(f);
    }
    return res;
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
