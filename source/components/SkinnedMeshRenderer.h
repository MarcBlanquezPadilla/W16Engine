#include "MeshRenderer.h"

class SkinnedMeshRenderer : public MeshRenderer
{
public:
    SkinnedMeshRenderer(GameObject* owner);

    void Update() override;

    ComponentType GetType() override { return ComponentType::SkinnedMeshRenderer; }
    bool IsType(ComponentType type) override { return type == ComponentType::SkinnedMeshRenderer || type == ComponentType::MeshRenderer;};


    AABB GetGlobalAABB() override;

    void SetMeshResource(UID uid) override;

    void LinkBones();
    void UpdateSkinningMatrices();
    void UpdateDynamicAABB();
    bool HasSkinning() const override { return hasSkinningData; }
    const std::vector<glm::mat4>& GetBoneMatrices() const { return cachedBoneMatrices; }

    void OnEvent(const Event& event) override;

    void OnEditor() override;

private:
    std::vector<GameObject*> boneGameObjects;
    std::vector<glm::mat4> cachedBoneMatrices;
    bool bonesLinked = false;
    bool hasSkinningData = false;
    AABB dynamicAABB;
};