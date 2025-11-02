#include "Texture.h"
#include "../Log.h"
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

bool Texture::LoadFromAssimpMaterial(aiMaterial* material, const std::string& modelDirectory)
{
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        aiString aiPath;
        material->GetTexture(aiTextureType_DIFFUSE, 0, &aiPath);

        std::string texPath = modelDirectory + aiPath.C_Str();

        if (LoadTexture(texPath)) 
        {
            return true;
        }
        else
        {
            LOG("Error: The Texture component could not load the texture from: %s", texPath.c_str());
            return false;
        }
    }
    else
    {
        LOG("The material does not have a diffuse texture.");
        return false;
    }
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

        ilBindImage(0);

        UploadToGPU();
        return true;
    }
    else
    {
        LOG("Error loading texture from: %s", path.c_str());
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

void Texture::OnDestroy()
{
    if (textureID != 0)
    {
        Engine::GetInstance().render->DeleteTextureFromGPU(textureID);
        textureID = 0;
    }

    UnloadFromCPU();
}