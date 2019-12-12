#include "resourcefile.hpp"
#include "global.hpp"
#include "platform/platform.hpp"
#include <cassert>

ResourceFile::ResourceFile(const std::string& name)
: Resource(name, ResourceType::File)
{

}

bool ResourceFile::Load()
{
    if (State::Init == mState)
    {
        if ( Global::platform()->Fetch(name().c_str()) )
            mState = State::Loaded;
        else
        {
            mState = State::Loading;
        }
    }
    return State::Loaded == mState;
}

void ResourceFile::SetLoadingSuccess(bool success)
{
    assert(State::Loading == mState);
    if(success)
    {
        mState = State::Loaded;
    }
    else
    {
        mState = State::Fail;
    }
}
