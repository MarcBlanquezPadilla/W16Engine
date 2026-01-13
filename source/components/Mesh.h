#pragma once
#include "Component.h"
#include "../Global.h"
#include "../EventListener.h"
#include "../utils/AABB.h"
#include "../resources/ResourceUser.h"

class ResourceMesh;
class GameObject;
class Config;

class Mesh : public Component, public ResourceUser, public EventListener
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
    UID GetMeshUID() const { return resourceUID; };
    AABB GetGlobalAABB();
    const std::vector<GameObject*>& GetBones() { return boneGameObjects; };
    void LinkBones();
    void UpdateSkinningMatrices();
    void UpdateDynamicAABB();
    const bool& HasSkinningData() { return hasSkinningData; };
    const std::vector<glm::mat4>& GetCachedBones() { return cachedBoneMatrices; };

    void OnResourceLost(UID resourceUID) override;
    void OnEvent(const Event& event) override;

public:

    bool drawNormals = false;
    bool drawMesh = false;
    bool drawStencil = false;

private:
    UID resourceUID = 0;
    mutable ResourceMesh* resource = nullptr;

    std::vector<GameObject*> boneGameObjects;
    std::vector<glm::mat4> cachedBoneMatrices;
    bool debugSkeleton = false;
    bool bonesLinked = false;
    bool hasSkinningData = false;
    bool cachedBones = false;

    AABB dynamicAABB;
};