#pragma once
#include "Resource.h"

class ResourceMesh : public Resource
{
public:
    ResourceMesh(uint32_t uid) : Resource(uid, ResourceType::MESH) {}
    virtual ~ResourceMesh();

    bool LoadInMemory() override;
    bool UnloadFromMemory_Internal() override;

public:
    uint32_t vao = 0;
    uint32_t vbo = 0;
    uint32_t ebo = 0;

    uint32_t numVertices = 0;
    uint32_t numIndices = 0;
};