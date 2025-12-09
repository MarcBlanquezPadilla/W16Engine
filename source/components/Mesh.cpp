#include "Mesh.h"
#include "../GameObject.h"
#include "../Engine.h"
#include "../ModuleResources.h"
#include "../resources/ResourceMesh.h"
#include "../components/Transform.h"
#include "../utils/Config.h"
#include "imgui.h"

Mesh::Mesh(GameObject* owner) : Component(owner)
{
}

Mesh::~Mesh()
{
    CleanUp();
}

void Mesh::Update(float dt)
{

}

void Mesh::CleanUp()
{
    if (meshUID != 0)
    {
        Engine::GetInstance().moduleResources->ReleaseResource(meshUID);
        meshUID = 0;
        resource = nullptr;
    }
}

void Mesh::Save(Config& componentNode)
{
    componentNode.SetUInt("MeshUID", meshUID);
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
    if (meshUID != 0)
    {
        Engine::GetInstance().moduleResources->ReleaseResource(meshUID);
    }

    meshUID = uid;
    resource = nullptr;

    if (meshUID != 0)
    {
        resource = (ResourceMesh*)Engine::GetInstance().moduleResources->RequestResource(meshUID);
    }
}

ResourceMesh* Mesh::GetResource() const
{
    if (resource == nullptr && meshUID != 0)
    {
        resource = (ResourceMesh*)Engine::GetInstance().moduleResources->RequestResource(meshUID);
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