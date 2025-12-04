#pragma once
#include "Resource.h"
#include "../utils/Config.h"

class ResourceScene : public Resource
{
public:
    ResourceScene(UID uid);
    virtual ~ResourceScene();

    bool LoadToMemory_Internal() override; 
    bool UnloadFromMemory_Internal() override;

public:
    Config sceneConfig;
};