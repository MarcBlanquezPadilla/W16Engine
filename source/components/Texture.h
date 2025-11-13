#pragma once
#include "Component.h"
#include <string>

struct aiMaterial;

class Texture : public Component
{
public:

    Texture(GameObject* owner, bool enabled);

    virtual ~Texture() override;

    void CleanUp() override;
    
    ComponentType GetType() override {
        return ComponentType::Texture;
    };

    void Save(pugi::xml_node componentNode) override;
    void Load(pugi::xml_node componentNode) override;

    bool LoadTexture(const std::string& path);

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
    bool transparent = false;
};