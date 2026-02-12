#pragma once
#include "Collider.h"

namespace physx { class PxConvexMesh; }

class ConvexCollider : public Collider {
public:
    ConvexCollider(GameObject* owner);
    ~ConvexCollider();

    void Update() override;

    physx::PxGeometry* GetGeometry() override;

    ColliderType GetColliderType() override { return ColliderType::CONVEX_COLLIDER; }
    ComponentType GetType() override { return ComponentType::ConvexCollider; }
    bool IsType(ComponentType type) override { return type == ComponentType::Collider || type == ComponentType::ConvexCollider; };

    void OnEditor() override;
    void Save(Config& config) override;
    void Load(Config& config) override;

    void DebugShape();

    void OnGameObjectEvent(GameObjectEvent event, Component* component) override;

private:
    void CookMesh();

    physx::PxConvexMesh* cookedMesh = nullptr;
};