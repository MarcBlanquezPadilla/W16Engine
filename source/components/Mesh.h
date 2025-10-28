#pragma once
#include "Component.h"

class GameObject;

class Mesh : public Component
{
public:

    Mesh(GameObject* owner, bool enabled);

    virtual ~Mesh() override;

    void Start() override;

    void Update(float deltaTime) override;

    void OnDestroy() override;

    void OnEnable() override;
    
    void OnDisable() override;
    
    ComponentType GetType() override;
};