#include "SkinnedMesh.h"
#include "../GameObject.h"
#include "../Engine.h"
#include "../ModuleResources.h"
#include "../ModuleEvents.h"
#include "../resources/ResourceMesh.h"
#include "../components/Transform.h"
#include "../utils/Config.h"
#include "../utils/LOG.h"
#include "imgui.h"

SkinnedMesh::SkinnedMesh(GameObject* owner) : Mesh(owner)
{
    Engine::GetInstance().moduleEvents->Subscribe(Event::Type::GameObjectDestroyed, this);
    bonesLinked = false;
}

void SkinnedMesh::Update()
{
    Mesh::Update();

    if (!bonesLinked && GetResource() != nullptr)
    {
        LinkBones();

        if (GetResource()->bones.empty() || !boneGameObjects.empty())
            bonesLinked = true;
    }
}

void SkinnedMesh::SetResource(UID uid)
{
    if (resource != nullptr)
    {
        Engine::GetInstance().moduleResources->ReleaseResource(resourceUID);
        resource->RemoveReference(this);
    }

    this->resourceUID = uid;

    Resource* res = Engine::GetInstance().moduleResources->RequestResource(uid);

    if (res != nullptr && res->GetType() == Resource::Type::mesh)
    {
        resource = (ResourceMesh*)res;
        resource->AddReference(this);
        bonesLinked = false;
        boneGameObjects.clear();
    }
    else
    {
        resource = nullptr;
    }
}

void SkinnedMesh::LinkBones()
{
    if (!resource) return;

    if (resource->bones.empty()) {
        boneGameObjects.clear();
        return;
    }

    boneGameObjects.clear();
    boneGameObjects.resize(resource->bones.size());

    GameObject* root = owner;
    for (int i = 0; root->parent != nullptr; i++) {
        root = root->parent;
    }

    for (size_t i = 0; i < resource->bones.size(); ++i)
    {
        std::string boneName = resource->bones[i].name;
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

 AABB SkinnedMesh::GetGlobalAABB()
{
     if(hasSkinningData) return dynamicAABB;
     return Mesh::GetGlobalAABB();

    return AABB();
}


void SkinnedMesh::OnEditor()
{

    if (ImGui::CollapsingHeader("Skinned Mesh"))
    {
        if (resource && resource->IsLoadedToMemory())
        {
            ImGui::Text("Mesh:");
            ImGui::Button(resource->GetName(), ImVec2(ImGui::GetContentRegionAvail().x, 20));
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(RESOURCE_DRAG))
                {
                    UID droppedUID = *(UID*)payload->Data;

                    const Resource* res = Engine::GetInstance().moduleResources->PeekResource(droppedUID);
                    if (res && res->GetType() == Resource::Type::mesh)
                    {
                        SetResource(droppedUID);
                    }
                }
                ImGui::EndDragDropTarget();
            }
            ImGui::Text("Path:");
            ImGui::TextWrapped(resource->GetAssetFile());
            ImGui::Text("Vertices:");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.0f, 1.0f), "%d", resource->numVertices);

            ImGui::Text("Indices:");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.0f, 1.0f), "%d", resource->numIndices);

            ImGui::Text("VAO (ID):");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.9f, 1.0f), "%u", resource->meshData.VAO);

            ImGui::Text("Has UVs:");
            ImGui::SameLine();
            ImGui::TextUnformatted(resource->hasUVs ? "Yes" : "No");
        }
        else
        {
            ImGui::Text("Mesh:");
            ImGui::Button("Drop mesh", ImVec2(ImGui::GetContentRegionAvail().x, 20));
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(RESOURCE_DRAG))
                {
                    UID droppedUID = *(UID*)payload->Data;

                    const Resource* res = Engine::GetInstance().moduleResources->PeekResource(droppedUID);
                    if (res && res->GetType() == Resource::Type::mesh)
                    {
                        SetResource(droppedUID);
                    }
                }
                ImGui::EndDragDropTarget();
            }
        }
    }
}

void SkinnedMesh::UpdateSkinningMatrices()
{
    if (cachedBones) return;

    if (!resource || resource->bones.empty()) {
        hasSkinningData = false;
        return;
    }

    const auto& bonesGO = boneGameObjects;
    size_t numBonesGO = bonesGO.size();

    if (bonesGO.empty()) {
        hasSkinningData = false;
        return;
    }

    const auto& resourceBonesGO = resource->bones;
    size_t numBonesResource = resource->bones.size();

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

void SkinnedMesh::UpdateDynamicAABB()
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

void SkinnedMesh::OnEvent(const Event& event)
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
