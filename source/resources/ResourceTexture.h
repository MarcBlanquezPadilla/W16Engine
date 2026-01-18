#pragma once
#include "Resource.h"

class ResourceTexture : public Resource
{
public:
    ResourceTexture(UID uid);
    ~ResourceTexture() override;

    bool LoadToMemory_Internal() override;
    bool UnloadFromMemory_Internal() override;

    unsigned int GetWidth() const { return width; }
    unsigned int GetHeight() const { return width; }
    unsigned int GetTextureGpuId() const { return textureGpuId; }

private:
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned int textureGpuId = 0;
};