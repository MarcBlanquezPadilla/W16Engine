#pragma once
#include "Resource.h"
#include "../utils/Config.h"

class ResourceScene : public Resource
{
public:
    ResourceScene(UID uid);
    ~ResourceScene() override;

    bool LoadToMemory_Internal() override; 
    bool UnloadFromMemory_Internal() override;

public:
    Config sceneConfig;
};