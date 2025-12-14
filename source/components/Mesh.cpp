#include "Mesh.h"
#include "../GameObject.h"
#include "../Engine.h"
#include "../ModuleResources.h"
#include "../resources/ResourceMesh.h"
#include "../components/Transform.h"
#include "../utils/Config.h"
#include "../utils/LOG.h"
#include "imgui.h"

Mesh::Mesh(GameObject* owner) : Component(owner)
{
}

Mesh::~Mesh()
{
    CleanUp();
}

void Mesh::Update()
{

}

void Mesh::CleanUp()
{
    if (resource)
    {
        Engine::GetInstance().moduleResources->ReleaseResource(resourceUID);
        resource->RemoveReference(this);
        resourceUID = 0;
        resource = nullptr;
    }
}

void Mesh::Save(Config& componentNode)
{
    componentNode.SetUInt("MeshUID", resourceUID);
    componentNode.SetBool("DrawNormals", drawNormals);
    componentNode.SetBool("DrawMesh", drawMesh);
    componentNode.SetBool("DrawStencil", drawStencil);
}

void Mesh::Load(Config& componentNode)
{
    UID uid = componentNode.GetUInt("MeshUID");
    drawNormals = componentNode.GetBool("DrawNormals");
    drawMesh = componentNode.GetBool("DrawMesh");
    drawStencil = componentNode.GetBool("DrawStencil");

    if (uid != 0) SetResource(uid);
}

void Mesh::SetResource(UID uid)
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
    }
    else
    {
        resource = nullptr;
    }
}

ResourceMesh* Mesh::GetResource() const
{
    if (resource == nullptr && resourceUID != 0)
    {
        resource = (ResourceMesh*)Engine::GetInstance().moduleResources->RequestResource(resourceUID);
    }
    return resource;
}

AABB Mesh::GetGlobalAABB()
{
    ResourceMesh* r = GetResource();
    if (r && r->IsLoadedToMemory())
    {
        glm::mat4 globalMatrix;

        if (owner->TryGetGlobalMatrix(globalMatrix))
        {
            return r->localAABB.GetGlobalAABB(globalMatrix);
        }
    }

    AABB empty;
    empty.SetNegativeInfinity();
    return empty;
}


void Mesh::OnEditor()
{

    if (ImGui::CollapsingHeader("Mesh"))
    {
        if (resource && resource->IsLoadedToMemory())
        {
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
        else ImGui::Text("There's no mesh attatched");
    }
}

void Mesh::OnResourceLost(UID lostUID)
{
    if (resourceUID == lostUID)
    {
        LOG("Texture Resource deleted! Removing reference in Component.");
        resource = nullptr;
        resourceUID = 0;
    }
}