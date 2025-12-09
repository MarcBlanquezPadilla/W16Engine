#pragma once
#include "Component.h"
#include "../Global.h"
#include "../utils/AABB.h"

class ResourceMesh;
class GameObject;
class Config;

class Mesh : public Component
{
public:
    Mesh(GameObject* owner);
    ~Mesh() override;

    void Update(float dt) override;
    void CleanUp() override;

    ComponentType GetType() override { return ComponentType::Mesh; }
    void Save(Config& componentNode) override;
    void Load(Config& componentNode) override;

    void OnEditor() override;

    void SetResource(UID uid);
    ResourceMesh* GetResource() const;
    UID GetMeshUID() const { return meshUID; }
    AABB GetGlobalAABB();

public:

    bool drawNormals = false;
    bool drawMesh = false;
    bool drawStencil = false;

private:
    UID meshUID = 0;
    mutable ResourceMesh* resource = nullptr;
};