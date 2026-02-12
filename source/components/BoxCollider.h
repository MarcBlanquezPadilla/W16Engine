#pragma once
#include "Collider.h"

class GameObject;

class BoxCollider : public Collider {
public:
    
    BoxCollider(GameObject* owner);



    physx::PxGeometry* GetGeometry() override;
    ColliderType GetColliderType() override { return ColliderType::BOX_COLLIDER; }
    ComponentType GetType() override { return ComponentType::BoxCollider; };
    bool IsType(ComponentType type) override { return type == ComponentType::Collider || type == ComponentType::BoxCollider; };
    void OnEditor() override;
    void Update() override;
    void Save(Config& config) override;
    void Load(Config& config) override;

    const glm::vec3& GetSize() { return size; }
    void SetSize(glm::vec3 size);

    void DebugShape() override;

private:
    glm::vec3 size = { 1.0f, 1.0f, 1.0f };
};