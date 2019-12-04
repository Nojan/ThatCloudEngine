#pragma once
#include <vector>

class Resource;

class IResourceOwner {
public:
    virtual void ListResources(std::vector<Resource*>& resources) = 0;
    virtual void OnLoad() = 0;
};

