#include "Texture.h"
#include "../utils/Log.h"
#include "Component.h"
#include <vector>
#include <assimp/scene.h>
#include "../Engine.h"
#include "../Render.h"
#include <IL/il.h>

Texture::Texture(GameObject* owner, bool enabled) : Component(owner, enabled)
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

    UnloadFromCPU();
}

void Texture::Save(pugi::xml_node componentNode)
{
    componentNode.append_attribute("type") = (int)GetType();
    componentNode.append_attribute("path") = path.c_str();
    componentNode.append_attribute("useChecker") = use_checker;
    componentNode.append_attribute("transparent") = transparent;
}

void Texture::Load(pugi::xml_node componentNode)
{
    path = componentNode.attribute("path").as_string();
    use_checker = componentNode.attribute("useChecker").as_bool();
    transparent = componentNode.attribute("transparent").as_bool();
    LoadTexture(path);
}

bool Texture::LoadTexture(const std::string& path)
{
    bool ret = true;

    this->path = path;

    ilGenImages(1, &ilImageID);
    ilBindImage(ilImageID);

    if (ilLoadImage(path.c_str()))
    {
        if (!ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE))
        {
            LOG("Error converting image to RGBA: %s", path.c_str());
            ilDeleteImages(1, &ilImageID);
            return false;
        }

        width = ilGetInteger(IL_IMAGE_WIDTH);
        height = ilGetInteger(IL_IMAGE_HEIGHT);

        LOG("Texture loaded into CPU from: %s (Width: %d, Height: %d)", path.c_str(), width, height);

        UploadToGPU();
        return true;
    }
    else
    {
        ilDeleteImages(1, &ilImageID);
        return false;
    }

  
    return ret;
}

void Texture::UploadToGPU()
{
    if (ilImageID == 0)
    {
        LOG("Error: An attempt was made to upload a texture to the GPU without first loading it to the CPU.");
        return;
    }

    ilBindImage(ilImageID);
    unsigned char* data = ilGetData();

    textureID = Engine::GetInstance().render->UploadTextureToGPU(
        data,
        width,
        height
    );

    ilBindImage(0);

    UnloadFromCPU();
}

void Texture::UnloadFromCPU()
{
    if (ilImageID != 0)
    {
        ilDeleteImages(1, &ilImageID);
        ilImageID = 0;
    }
}