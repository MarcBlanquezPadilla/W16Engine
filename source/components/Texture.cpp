#include "Texture.h"
#include "../utils/Log.h"
#include "Component.h"
#include <vector>
#include <assimp/scene.h>
#include "../Engine.h"
#include "../Render.h"
#include "../Loader.h"
#include "imgui.h"

Texture::Texture(GameObject* owner) : Component(owner)
{
	
}

Texture::~Texture()
{
    
}

void Texture::CleanUp()
{
    if (textureID != 0)
    {
        Engine::GetInstance().render->DeleteTextureFromGPU(textureID);
        textureID = 0;
    }

}

void Texture::Save(pugi::xml_node componentNode)
{
    componentNode.append_attribute("path") = path.c_str();
    componentNode.append_attribute("useChecker") = use_checker;
    componentNode.append_attribute("transparent") = transparent;
}

void Texture::Load(pugi::xml_node componentNode)
{
    path = componentNode.attribute("path").as_string();
    use_checker = componentNode.attribute("useChecker").as_bool();
    transparent = componentNode.attribute("transparent").as_bool();
    Engine::GetInstance().loader->LoadTextureToGameObject(path, this->owner);
}

void Texture::SetTexture(const std::string p, unsigned int t, int w, int h)
{
    path = p;
    textureID = t;
    width = w;
    height = h;
}


void Texture::OnEditor()
{
    if (ImGui::CollapsingHeader("Texture"))
    {
        ImGui::Text("Path:");
        ImGui::TextWrapped(path.c_str());

        ImGui::Text("Size:");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.9f, 1.0f), "%dx%d", width, height);
        ImGui::Text("Texture ID (GPU):");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.9f, 1.0f), "%u", textureID);
        ImGui::Checkbox("Use Checker Texture", &use_checker);
        ImGui::Checkbox("Transparent", &transparent);
    }
}