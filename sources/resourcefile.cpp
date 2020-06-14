#include "resourcefile.hpp"
#include "global.hpp"
#include "platform/platform.hpp"
#include <cassert>

ResourceFile::ResourceFile(const std::string& name)
: Resource(name, ResourceType::File)
{

}

ResourceFile::State ResourceFile::GetState() const
{
    return mState;
}

bool ResourceFile::Load(Resource* owner)
{
    if (State::Init == mState)
    {
        if ( Global::platform()->Fetch(name().c_str()) )
        {
            mState = State::Loaded;
            owner->OnDependencyLoad(this);
        }
        else
        {
            mState = State::Loading;
        }
    }
    const bool isLoaded = State::Loaded == mState;
    if (!isLoaded && owner)
    {
        mOwners.push_back(owner);
    }
    return isLoaded;
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
    for (auto& owner : mOwners)
    {
        owner->OnDependencyLoad(this);
    }
    mOwners.resize(0);
}
