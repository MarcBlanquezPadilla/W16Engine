#pragma once
#include "pugixml.hpp"
class GameObject;

enum class ComponentType {
    None,
    Transform,
    Mesh,
    Texture,
    Camera
};

class Component
{
public:

    Component(GameObject* owner) : owner(owner) {}

    virtual ~Component() {}

    virtual void Start() {}
    
    virtual void OnEnable() {}
    
    virtual void OnDisable() {}

    virtual void Update(float deltaTime) {}

    virtual void CleanUp() {}
    
    virtual ComponentType GetType() { return ComponentType::None; };

    virtual void Save(pugi::xml_node componentNode) {}

    virtual void Load(pugi::xml_node componentNode) {}

    virtual void OnEditor() {}

public:
    GameObject* owner;
    bool enabled = true;
};