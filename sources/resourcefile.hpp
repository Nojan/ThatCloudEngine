#pragma once
#include "resource.hpp"

class ResourceFile : public Resource
{
public:
    enum class State {
        Init,
        Loading,
        Loaded,
        Fail,
    };
    ResourceFile(const std::string& name);

    bool Load(Resource* owner = nullptr) override;
    void SetLoadingSuccess(bool success);
private:
    State mState = State::Init;
};
