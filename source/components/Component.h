#pragma once
#include "pugixml.hpp"
class GameObject;

enum class ComponentType {
    None,
    Transform,
    Mesh,
    Texture
};

class Component
{
public:

    Component(GameObject* owner, bool enabled) : owner(owner), enabled(enabled) {}

    virtual ~Component() {}

    virtual void Start() {}

    virtual void Update(float deltaTime) {}

    virtual void OnDestroy() {}

    virtual void OnEnable() {}
    
    virtual void OnDisable() {}
    
    virtual ComponentType GetType() { return ComponentType::None; };

    virtual void Save(pugi::xml_node componentNode) {}

    virtual void Load(pugi::xml_node componentNode) {}

public:
    GameObject* owner;
    bool enabled;
};