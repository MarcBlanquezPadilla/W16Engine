#include "Mesh.h"
#include "../GameObject.h"
#include "../Engine.h"
#include "../ModuleResources.h"
#include "../ModuleEvents.h"
#include "../resources/ResourceMesh.h"
#include "../components/Transform.h"
#include "../utils/Config.h"
#include "../utils/LOG.h"
#include "imgui.h"

Mesh::Mesh(GameObject* owner) : Component(owner)
{
    Engine::GetInstance().moduleEvents->Subscribe(Event::Type::GameObjectDestroyed, this);
}

Mesh::~Mesh()
{
    CleanUp();
}

void Mesh::Update()
{

    if (!bonesLinked)
    {
        if (resource != nullptr)
        {
            LinkBones(); // <--- LLAMADA CLAVE

            if (!boneGameObjects.empty() || resource->bones.empty())
            {
                bonesLinked = true;
            }
        }
    }
    else
    {
        cachedBones = false;
    }
}

void Mesh::CleanUp()
{
    Engine::GetInstance().moduleEvents->Unsubscribe(Event::Type::GameObjectDestroyed,this);
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
        bonesLinked = false;
        boneGameObjects.clear();
    }
    else
    {
        resource = nullptr;
    }
}

void Mesh::LinkBones()
{
    if (!resource) return;

    // Si el resource no tiene huesos, no hacemos nada
    if (resource->bones.empty()) {
        boneGameObjects.clear();
        return;
    }

    // 1. Limpiamos y preparamos el vector
    boneGameObjects.clear();
    boneGameObjects.resize(resource->bones.size());

    // 2. Buscamos la raíz del modelo (subimos hasta encontrar el Animator o el tope)
    GameObject* root = owner;
    for (int i = 0; root->parent != nullptr; i++) {
        root = root->parent;
        // Si tuvieras un componente Animator, podrías parar aquí.
    }

    // 3. Enlazamos los nombres del Resource con los GameObjects de la Scene [cite: 9]
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
            // Si no lo encuentra, null (y luego pondremos matriz identidad)
            boneGameObjects[i] = nullptr;
            LOG(LogType::LOG_WARNING, "Bone '%s' not found for mesh '%s'", boneName.c_str(), owner->name.c_str());
        }
    }
    LOG(LogType::LOG_INFO, "Skinning: Linked %d bones for mesh %s", boneGameObjects.size(), owner->name.c_str());
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

        if (owner->GetGlobalMatrix(globalMatrix))
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
        LOG(LogType::LOG_INFO, "Texture Resource deleted! Removing reference in Component.");
        resource = nullptr;
        resourceUID = 0;
    }
}

void Mesh::UpdateSkinningMatrices()
{
    if (cachedBones) return;

    if (!resource || resource->bones.empty() || GetBones().empty()) {
        hasSkinningData = false;
        return;
    }

    if (cachedBoneMatrices.size() < resource->bones.size()) {
        cachedBoneMatrices.resize(resource->bones.size());
    }

    glm::mat4 globalMatrix = owner->transform->GetGlobalMatrix();
    glm::mat4 meshInverseTransform = glm::inverse(globalMatrix);

    for (size_t i = 0; i < GetBones().size(); ++i)
    {
        GameObject* boneGO = GetBones()[i];
        if (boneGO)
        {
            Transform* trans = (Transform*)boneGO->transform;
            cachedBoneMatrices[i] = meshInverseTransform * trans->GetGlobalMatrix() * resource->bones[i].offsetMatrix;
        }
        else
        {
            cachedBoneMatrices[i] = glm::mat4(1.0f);
        }
    }

    hasSkinningData = true;
}

void Mesh::OnEvent(const Event& event)
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
