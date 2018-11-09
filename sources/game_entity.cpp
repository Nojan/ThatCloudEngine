#include "game_entity.hpp"

#include "global.hpp"
#include <cassert>

GameEntity::~GameEntity()
{
}

void GameEntity::addUntypedComponent(std::type_index index, void * untypedPointer)
{
    assert(nullptr != untypedPointer);
    assert(nullptr == components[index]);
    components[index] = untypedPointer;
    assert(untypedPointer == components[index]);
}

void GameEntity::removeUntypedComponent(std::type_index index)
{
    assert(nullptr != components[index]);
    components.erase(index);
}

void* GameEntity::getUntypedComponent(std::type_index index)
{
    auto it = components.find(index);
    if(components.end() == it)
        return nullptr;
    return it->second;
}

