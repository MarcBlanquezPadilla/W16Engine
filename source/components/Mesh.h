#pragma once
#include "Component.h"
#include "../Global.h"
#include "../utils/AABB.h"
#include "../resources/ResourceUser.h"

class ResourceMesh;
class GameObject;
class Config;

class Mesh : public Component, public ResourceUser
{
public:
    Mesh(GameObject* owner);
    ~Mesh() override;

    void Update() override;
    void CleanUp() override;

    ComponentType GetType() override { return ComponentType::Mesh; }
    void Save(Config& componentNode) override;
    void Load(Config& componentNode) override;

    void OnEditor() override;

    void SetResource(UID uid);
    ResourceMesh* GetResource() const;
    UID GetMeshUID() const { return resourceUID; }
    AABB GetGlobalAABB();
    const std::vector<GameObject*>& GetBones() { return boneGameObjects; };
    void LinkBones();

    void OnResourceLost(UID resourceUID) override;

public:

    bool drawNormals = false;
    bool drawMesh = false;
    bool drawStencil = false;

private:
    UID resourceUID = 0;
    mutable ResourceMesh* resource = nullptr;

    std::vector<GameObject*> boneGameObjects;
    bool debugSkeleton = false;
    bool bonesLinked = false;
};