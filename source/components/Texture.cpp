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

        if (this->LoadTexture(texPath)) 
        {
            this->UploadToGPU();
            return true;
        }
        else
        {
            LOG("Error: El componente Texture no pudo cargar la textura desde: %s", texPath.c_str());
            return false;
        }
    }
    else
    {
        LOG("El material no tiene textura difusa.");
        return false;
    }
}

bool Texture::LoadTexture(const std::string& path)
{
    bool ret = true;

    this->path = path;

    ilGenImages(1, &this->ilImageID);
    ilBindImage(this->ilImageID);

    if (ilLoadImage(path.c_str()))
    {
        if (!ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE))
        {
            LOG("Error al convertir la imagen a RGBA: %s", path.c_str());
            ilDeleteImages(1, &this->ilImageID);
            return false;
        }

        this->width = ilGetInteger(IL_IMAGE_WIDTH);
        this->height = ilGetInteger(IL_IMAGE_HEIGHT);

        LOG("Textura cargada en CPU desde: %s (Ancho: %d, Alto: %d)", path.c_str(), width, height);

        ilBindImage(0);
        return true;
    }
    else
    {
        LOG("Error al cargar la textura desde: %s", path.c_str());
        ilDeleteImages(1, &this->ilImageID);
        return false;
    }

    return ret;
}

void Texture::UploadToGPU()
{
    if (this->ilImageID == 0)
    {
        LOG("Error: Se intentó subir textura a GPU sin cargarla en CPU primero.");
        return;
    }

    ilBindImage(this->ilImageID);
    unsigned char* data = ilGetData();

    this->textureID = Engine::GetInstance().render->UploadTextureToGPU(
        data,
        this->width,
        this->height
    );

    ilBindImage(0);

    UnloadFromCPU();
}

void Texture::UnloadFromCPU()
{
    if (this->ilImageID != 0)
    {
        ilDeleteImages(1, &this->ilImageID);
        this->ilImageID = 0;
        this->width = 0;
        this->height = 0;
    }
}

void Texture::OnDestroy()
{
    if (this->textureID != 0)
    {
        Engine::GetInstance().render->DeleteTextureFromGPU(this->textureID);
        this->textureID = 0;
    }

    UnloadFromCPU();
}