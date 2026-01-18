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
    virtual ~Mesh() override;

    void Update() override { cachedBones = false; }
    void CleanUp() override;

    virtual ComponentType GetType() override { return ComponentType::Mesh; }
    bool IsType(ComponentType type) override { return type == ComponentType::Mesh; };
    void Save(Config& componentNode) override;
    void Load(Config& componentNode) override;

    virtual void OnEditor() override;

    virtual void SetResource(UID uid);
    ResourceMesh* GetResource() const;

    virtual AABB GetGlobalAABB();

    //SKINNING
    virtual void UpdateSkinningMatrices() {}
    virtual void UpdateDynamicAABB() {}
    virtual bool HasSkinning() const { return false; }
    virtual const std::vector<glm::mat4>& GetBoneMatrices() const { static std::vector<glm::mat4> empty; return empty; }
    
    void OnResourceLost(UID resourceUID) override;
    virtual void OnEvent(const Event& event) override {};

public:
    bool drawNormals = false;
    bool drawMesh = false;
    bool drawStencil = false;

protected:
    UID resourceUID = 0;
    mutable ResourceMesh* resource = nullptr;
    bool cachedBones = false;
};