#include "SkinnedMeshRenderer.h"
#include "../GameObject.h"
#include "../Engine.h"
#include "../ModuleResources.h"
#include "../ModuleEvents.h"
#include "../resources/ResourceMesh.h"
#include "../components/Transform.h"
#include "../utils/Config.h"
#include "../utils/LOG.h"
#include "imgui.h"

SkinnedMeshRenderer::SkinnedMeshRenderer(GameObject* owner) : MeshRenderer(owner)
{
    Engine::GetInstance().moduleEvents->Subscribe(Event::Type::GameObjectDestroyed, this);
    bonesLinked = false;
}

void SkinnedMeshRenderer::Update()
{
    MeshRenderer::Update();

    if (!bonesLinked && GetMeshResource() != nullptr)
    {
        LinkBones();

        if (GetMeshResource()->bones.empty() || !boneGameObjects.empty())
            bonesLinked = true;
    }
}

void SkinnedMeshRenderer::SetMeshResource(UID uid)
{
    if (meshResource != nullptr)
    {
        Engine::GetInstance().moduleResources->ReleaseResource(meshResource->GetUID());
        meshResource->RemoveReference(this);
    }

    meshUID = uid;
    Resource* res = Engine::GetInstance().moduleResources->RequestResource(uid);

    if (res != nullptr && res->GetType() == Resource::Type::mesh)
    {
        meshResource = (ResourceMesh*)res;
        meshResource->AddReference(this);
        bonesLinked = false;
        boneGameObjects.clear();
    }
    else
    {
        meshResource = nullptr;
    }
}

void SkinnedMeshRenderer::LinkBones()
{
    if (!meshResource) return;

    if (meshResource->bones.empty()) {
        boneGameObjects.clear();
        return;
    }

    boneGameObjects.clear();
    boneGameObjects.resize(meshResource->bones.size());

    GameObject* root = owner;
    for (int i = 0; root->parent != nullptr; i++) {
        root = root->parent;
    }

    for (size_t i = 0; i < meshResource->bones.size(); ++i)
    {
        std::string boneName = meshResource->bones[i].name;
        GameObject* foundBone = root->FindChild(boneName);

        if (foundBone)
        {
            boneGameObjects[i] = foundBone;
        }
        else
        {
            boneGameObjects[i] = nullptr;
            LOG(LogType::LOG_WARNING, "Bone '%s' not found for mesh '%s'", boneName.c_str(), owner->name.c_str());
        }
    }
    LOG(LogType::LOG_INFO, "Skinning: Linked %d bones for mesh %s", boneGameObjects.size(), owner->name.c_str());
}

 AABB SkinnedMeshRenderer::GetGlobalAABB()
{
     if(hasSkinningData) return dynamicAABB;
     return MeshRenderer::GetGlobalAABB();

    return AABB();
}


void SkinnedMeshRenderer::OnEditor()
{

    if (ImGui::CollapsingHeader("Skinned Mesh Renderer"))
    {
        OnEditorMesh();
        ImGui::Separator();
        OnEditorTexture();
    }
}

void SkinnedMeshRenderer::UpdateSkinningMatrices()
{
    if (cachedBones) return;

    if (!meshResource || meshResource->bones.empty()) {
        hasSkinningData = false;
        return;
    }

    const auto& bonesGO = boneGameObjects;
    size_t numBonesGO = bonesGO.size();

    if (bonesGO.empty()) {
        hasSkinningData = false;
        return;
    }

    const auto& resourceBonesGO = meshResource->bones;
    size_t numBonesResource = meshResource->bones.size();

    if (cachedBoneMatrices.size() != numBonesResource) {
        cachedBoneMatrices.resize(numBonesResource);
    }

    glm::mat4 globalMatrix = owner->transform->GetGlobalMatrix();
    glm::mat4 meshInverseTransform = glm::inverse(globalMatrix);

    for (size_t i = 0; i < numBonesResource; ++i)
    {
        if (i < numBonesGO && bonesGO[i] != nullptr)
        {
            Transform* t = (Transform*)bonesGO[i]->transform;

            cachedBoneMatrices[i] = meshInverseTransform * t->GetGlobalMatrix() * resourceBonesGO[i].offsetMatrix;
        }
        else
        {
            cachedBoneMatrices[i] = glm::mat4(1.0f);
        }
    }

    hasSkinningData = true;
}

void SkinnedMeshRenderer::UpdateDynamicAABB()
{
    if (!hasSkinningData || boneGameObjects.empty()) return;

    glm::vec3 minP(INFINITY);
    glm::vec3 maxP(-INFINITY);

    for (GameObject* bone : boneGameObjects)
    {
        if (bone && bone->transform)
        {
            glm::vec3 pos = bone->transform->GetGlobalPosition();
            minP = glm::min(minP, pos);
            maxP = glm::max(maxP, pos);
        }
    }

    float padding = AABB_PADDING;
    dynamicAABB.min = minP - glm::vec3(padding);
    dynamicAABB.max = maxP + glm::vec3(padding);
}

void SkinnedMeshRenderer::OnEvent(const Event& event)
{
    switch (event.type)
    {
    case Event::Type::GameObjectDestroyed:
    {
        if (boneGameObjects.empty()) return;

        GameObject* deletedGO = event.data.gameObject.gameObject;

        for (size_t i = 0; i < boneGameObjects.size(); ++i)
        {
            if (boneGameObjects[i] == deletedGO)
            {
                boneGameObjects[i] = nullptr;
            }
        }

        break;
    }

    default:
        break;
    }
}
