#pragma once
#include "Collider.h"

class InfinitePlaneCollider : public Collider {
public:
    InfinitePlaneCollider(GameObject* owner);
    virtual ~InfinitePlaneCollider() = default;

    void Update() override;

    bool CanBeDynamic() const override { return false; }

    ColliderType GetColliderType() override { return ColliderType::INFINITE_PLANE_COLLIDER; }
    ComponentType GetType() override { return ComponentType::InfinitePlaneCollider; }
    bool IsType(ComponentType type) override { return type == ComponentType::Collider || type == ComponentType::InfinitePlaneCollider; };

    physx::PxGeometry* GetGeometry() override;

    void Save(Config& config) override;
    void Load(Config& config) override;

    void OnEditor() override;
    void DebugShape() override;
};