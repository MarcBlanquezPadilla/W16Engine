#pragma once
#include "Resource.h"
#include "../utils/Config.h"


class ResourceModel : public Resource
{
public:
    ResourceModel(UID uid);
    ~ResourceModel() override;

    bool LoadToMemory_Internal() override;
    bool UnloadFromMemory_Internal() override;

public:
    Config gameObjectConfig;
};