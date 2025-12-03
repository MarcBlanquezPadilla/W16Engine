#include "../Engine.h"
#include "../ModuleRender.h"
#include "../ModuleResources.h"

#include "Component.h"
#include "Texture.h"

#include "../resources/Resource.h"
#include "../resources/ResourceTexture.h"

#include "../utils/Log.h"

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
    if (textureUID != 0)
    {
        Engine::GetInstance().moduleResources->ReleaseResource(textureUID);
        textureUID = 0;
        resource = nullptr;
    }
}

void Texture::Save(pugi::xml_node componentNode)
{
    componentNode.append_attribute("textureUID").set_value(textureUID);
    componentNode.append_attribute("useChecker").set_value(use_checker);
    componentNode.append_attribute("transparent").set_value(transparent);
}

void Texture::Load(pugi::xml_node componentNode)
{
    UID uid = componentNode.attribute("textureUID").as_uint();
    use_checker = componentNode.attribute("useChecker").as_bool();
    transparent = componentNode.attribute("transparent").as_bool();

    if (uid != 0) SetResource(uid);
}

void Texture::SetResource(UID uid)
{
    if (textureUID != 0)
    {
        Engine::GetInstance().moduleResources->ReleaseResource(textureUID);
    }

    textureUID = uid;
    resource = nullptr;

    if (textureUID != 0)
    {
        GetResource();
    }
}

ResourceTexture* Texture::GetResource() const
{
    if (resource == nullptr && textureUID != 0)
    {
        resource = (ResourceTexture*)Engine::GetInstance().moduleResources->RequestResource(textureUID);
    }
    return resource;
}

unsigned int Texture::GetTextureID() const
{
    if (use_checker) return 0;

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
        ImGui::Text("Path:");
        ImGui::TextWrapped(resource->GetAssetFile());

        ImGui::Text("Size:");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.9f, 1.0f), "%dx%d", GetTextureWidth(), GetTextureHeight());
        ImGui::Text("Texture ID (GPU):");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.9f, 1.0f), "%u", GetTextureID());
        ImGui::Checkbox("Use Checker Texture", &use_checker);
        ImGui::Checkbox("Transparent", &transparent);
    }
}