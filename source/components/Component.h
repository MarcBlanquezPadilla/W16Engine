#pragma once
#include "../utils/Config.h"
#include "../GameObject.h"


enum class ComponentRole
{

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
    
    virtual void FixedUpdate() {}

    virtual void CleanUp() {}
    
    virtual ComponentType GetType() = 0;
    
    virtual bool IsType(ComponentType type) = 0;
    
    virtual bool IsIncompatible(ComponentType type) = 0;

    virtual void Save(Config& componentNode) {}

    virtual void Load(Config& componentNode) {}

    virtual void OnEditor() {}

    void SetEnabled(bool enabled) { this->enabled = enabled; }
    
    const bool GetEnabled() { return enabled; }

    virtual void OnGameObjectEvent(GameObjectEvent event, Component* component) {};


public:
    std::string name;
    GameObject* owner;

private:
    bool enabled = true;
};