#pragma once
#include "Component.h"
#include "../Global.h"
#include <string>

struct aiMaterial;
class ResourceTexture;

class Texture : public Component
{
public:

    Texture(GameObject* owner);

    virtual ~Texture() override;

    void CleanUp() override;
    
    ComponentType GetType() override {
        return ComponentType::Texture;
    };

    void Save(pugi::xml_node componentNode) override;
    void Load(pugi::xml_node componentNode) override;

    void OnEditor() override;

    void SetResource(UID uid);
   
    ResourceTexture* GetResource() const;

    unsigned int GetTextureID() const;
    unsigned int GetTextureWidth() const;
    unsigned int GetTextureHeight() const;

public:

    UID textureUID = 0;

    bool use_checker = false;
    bool transparent = false;

private:
    
    mutable ResourceTexture* resource = nullptr;
};