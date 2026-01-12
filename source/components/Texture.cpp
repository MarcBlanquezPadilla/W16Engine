#include "../Engine.h"
#include "../ModuleRender.h"
#include "../ModuleResources.h"
#include "../ModuleEvents.h"

#include "Component.h"
#include "Texture.h"

#include "../resources/Resource.h"
#include "../resources/ResourceTexture.h"

#include "../utils/Log.h"
#include "../utils/FileUtils.h"

#include "imgui.h"

#include <vector>
#include <assimp/scene.h>

Texture::Texture(GameObject* owner) : Component(owner)
{
	
}

Texture::~Texture()
{
   
}

void Texture::CleanUp()
{
    if (resource != nullptr)
    {
        resource->RemoveReference(this);
        Engine::GetInstance().moduleResources->ReleaseResource(resourceUID);
        resourceUID = 0;
        resource = nullptr;
    }
}

void Texture::Save(Config& componentNode)
{
    componentNode.SetUInt("textureUID", resourceUID);
    componentNode.SetBool("useChecker", use_checker);
    componentNode.SetBool("transparent", transparent);
}

void Texture::Load(Config& componentNode)
{
    UID uid = componentNode.GetUInt("textureUID");
    use_checker = componentNode.GetBool("useChecker");
    transparent = componentNode.GetBool("transparent");

    if (uid != 0) SetResource(uid);
}

void Texture::SetResource(UID uid)
{
    if (resource != nullptr)
    {
        Engine::GetInstance().moduleResources->ReleaseResource(resourceUID);
        resource->RemoveReference(this);
    }

    this->resourceUID = uid;

    Resource* res = Engine::GetInstance().moduleResources->RequestResource(uid);

    if (res != nullptr && res->GetType() == Resource::Type::texture)
    {
        resource = (ResourceTexture*)res;

        resource->AddReference(this);
    }
    else
    {
        resource = nullptr;
    }
}

ResourceTexture* Texture::GetResource() const
{
    if (resource == nullptr && resourceUID != 0)
    {
        resource = (ResourceTexture*)Engine::GetInstance().moduleResources->RequestResource(resourceUID);
    }
    return resource;
}

unsigned int Texture::GetTextureID() const
{
    if (resource && resource->IsLoadedToMemory())
    {
        return resource->gpuID;
    }
    return 0;
}

unsigned int Texture::GetTextureWidth() const
{
    if (resource && resource->IsLoadedToMemory())
    {
        return resource->width;
    }
    return 0;
}

unsigned int Texture::GetTextureHeight() const
{
    if (resource && resource->IsLoadedToMemory())
    {
        return resource->height;
    }
    return 0;
}

void Texture::OnEditor()
{
    if (ImGui::CollapsingHeader("Texture"))
    {
        if (resource && resource->IsLoadedToMemory())
        {
            ImGui::Text("Texture:");
            ImGui::Image(GetTextureID(), ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().x));

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(ASSETS_DRAG))
                {
                    const char* dataPtr = (const char*)payload->Data;
                    const char* endPtr = dataPtr + payload->DataSize;

                    while (dataPtr < endPtr)
                    {
                        std::string path = dataPtr;
                        if (path.empty()) break;

                        if (!IsFileDirectory(path))
                        {
                            Resource::Type type = Engine::GetInstance().moduleResources->GetTypeFromExtension(path);
                            if (type == Resource::texture)
                            {
                                UID droppedUID = Engine::GetInstance().moduleResources->Find(path);
                                const Resource* res = Engine::GetInstance().moduleResources->PeekResource(droppedUID);

                                if (res && res->GetType() == Resource::Type::texture)
                                {
                                    SetResource(droppedUID);
                                }
                            }
                        }

                        dataPtr += path.length() + 1;
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::Text("Path:");
            ImGui::TextWrapped(resource->GetAssetFile());
            ImGui::Text("Size:");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.9f, 1.0f), "%dx%d", GetTextureWidth(), GetTextureHeight());
        }
        else
        {
            ImGui::Text("Texture:");
            ImGui::Button("Drop texture", ImVec2(ImGui::GetContentRegionAvail().x, 20));

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(RESOURCE_DRAG))
                {
                    UID droppedUID = *(UID*)payload->Data;

                    const Resource* res = Engine::GetInstance().moduleResources->PeekResource(droppedUID);
                    if (res && res->GetType() == Resource::Type::texture)
                    {
                        SetResource(droppedUID);
                    }
                }
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(ASSETS_DRAG))
                {
                    const char* dataPtr = (const char*)payload->Data;
                    const char* endPtr = dataPtr + payload->DataSize;

                    while (dataPtr < endPtr)
                    {
                        std::string path = dataPtr;
                        if (path.empty()) break;

                        if (!IsFileDirectory(path))
                        {
                            Resource::Type type = Engine::GetInstance().moduleResources->GetTypeFromExtension(path);
                            if (type == Resource::texture)
                            {
                                UID droppedUID = Engine::GetInstance().moduleResources->Find(path);
                                const Resource* res = Engine::GetInstance().moduleResources->PeekResource(droppedUID);

                                if (res && res->GetType() == Resource::Type::texture)
                                {
                                    SetResource(droppedUID);
                                }
                            }
                        }

                        dataPtr += path.length() + 1;
                    }
                }
                ImGui::EndDragDropTarget();
            }
            
            ImGui::Text("Path:");
            ImGui::TextWrapped("Default texture");
        }
       
        ImGui::Checkbox("Use Checker Texture", &use_checker);
        ImGui::Checkbox("Transparent", &transparent);
    }
}



void Texture::OnResourceLost(UID lostUID)
{
    if (resourceUID == lostUID)
    {
        LOG(LogType::LOG_INFO, "Texture Resource deleted! Removing reference in Component.");
        this->resource = nullptr;
    }
}