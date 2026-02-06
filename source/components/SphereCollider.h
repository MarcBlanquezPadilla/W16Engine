#pragma once
#include "Collider.h"

class GameObject;

class SphereCollider : public Collider {
public:
    
    SphereCollider(GameObject* owner);

    float radius = 1.0f;

    physx::PxGeometry* GetGeometry() override;
    ColliderType GetColliderType() override { return ColliderType::SPHERE_COLLIDER; }
    ComponentType GetType() override { return ComponentType::SphereCollider; };
    bool IsType(ComponentType type) override { return type == ComponentType::Collider || type == ComponentType::SphereCollider; };
    void OnEditor() override;
    void Save(Config& config) override;
    void Load(Config& config) override;

    const float GetRadius() { return radius; }
    void SetRadius(float radius);
};