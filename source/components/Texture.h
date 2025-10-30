#pragma once
#include "Component.h"
#include <string>

struct aiMaterial;

class Texture : public Component
{
public:

    Texture(GameObject* owner, bool enabled);

    virtual ~Texture() override;

    void OnDestroy() override;
    
    ComponentType GetType() override {
        return ComponentType::Texture;
    };


    bool LoadTexture(const std::string& path);

    bool LoadFromAssimpMaterial(aiMaterial* material, const std::string& modelDirectory);

    void UploadToGPU();

    void UnloadFromCPU();

    unsigned int GetTextureID() const { return textureID; }

public:

    std::string path;
    unsigned int textureID = 0;
    unsigned int ilImageID = 0;
    int width = 0;
    int height = 0;
    bool use_checker = false;
};