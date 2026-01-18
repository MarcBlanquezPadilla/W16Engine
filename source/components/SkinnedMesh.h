#include "Mesh.h"

class SkinnedMesh : public Mesh
{
public:
    SkinnedMesh(GameObject* owner);

    void Update() override;
    ComponentType GetType() override { return ComponentType::SkinnedMesh; }
    bool IsType(ComponentType type) override { return type == ComponentType::SkinnedMesh || type == ComponentType::Mesh;};

    void OnEditor() override;
    AABB GetGlobalAABB() override;

    void SetResource(UID uid) override;
    void LinkBones();
    void UpdateSkinningMatrices();
    void UpdateDynamicAABB();

    bool HasSkinning() const override { return hasSkinningData; }
    const std::vector<glm::mat4>& GetBoneMatrices() const override { return cachedBoneMatrices; }

    void OnEvent(const Event& event) override;

private:
    std::vector<GameObject*> boneGameObjects;
    std::vector<glm::mat4> cachedBoneMatrices;
    bool bonesLinked = false;
    bool hasSkinningData = false;
    AABB dynamicAABB;
};