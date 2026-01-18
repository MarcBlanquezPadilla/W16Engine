#pragma once
#include "../utils/Config.h"
class GameObject;

enum class ComponentType {
    None,
    Transform,
    Texture,
    MeshRenderer,
    SkinnedMeshRenderer,
    Camera,
    Animation
};

class Component
{
public:

    Component(GameObject* owner) : owner(owner) {}

    virtual ~Component() {}

    virtual void Start() {}
    
    virtual void OnEnable() {}
    
    virtual void OnDisable() {}

    virtual void Update() {}

    virtual void CleanUp() {}
    
    virtual ComponentType GetType() = 0;
    
    virtual bool IsType(ComponentType type) = 0;

    virtual void Save(Config& componentNode) {}

    virtual void Load(Config& componentNode) {}

    virtual void OnEditor() {}

    void SetEnabled(bool enabled) { this->enabled = enabled; }
    
    const bool& GetEnabled() { return enabled; }

public:
    GameObject* owner;

private:
    bool enabled = true;
};