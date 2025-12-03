#pragma once
#include "Resource.h"

class ResourceTexture : public Resource
{
public:
    ResourceTexture(UID uid);
    virtual ~ResourceTexture();

    bool LoadToMemory_Internal() override;
    bool UnloadFromMemory_Internal() override;

public:
    unsigned int width = 0;
    unsigned int height = 0;

    unsigned int gpuID = 0;
};