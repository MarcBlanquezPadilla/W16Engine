#pragma once
#include "Component.h"
#include "../Global.h"
#include "../EventListener.h"
#include "../resources/ResourceUser.h"
#include <string>

struct aiMaterial;
class ResourceTexture;

class Texture : public Component, public ResourceUser
{
public:

    Texture(GameObject* owner);

    virtual ~Texture() override;

    void CleanUp() override;
    
    ComponentType GetType() override { return ComponentType::Texture; };
    bool IsType(ComponentType type) override { return type == ComponentType::Texture; };

    void Save(Config& componentNode) override;
    void Load(Config& componentNode) override;

    void OnEditor() override;

    void SetResource(UID uid);
   
    ResourceTexture* GetResource() const;

    unsigned int GetTextureID() const;
    unsigned int GetTextureWidth() const;
    unsigned int GetTextureHeight() const;

    void OnResourceLost(UID resourceUID) override;

public:

    UID resourceUID = 0;

    bool use_checker = false;
    bool transparent = false;

private:
    
    mutable ResourceTexture* resource = nullptr;
};