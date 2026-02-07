#pragma once
#include "Component.h"
#include <glm/glm.hpp>
#include "glm/gtc/quaternion.hpp"
#include "../utils/Log.h"
#include "PhysicsEventsListener.h"

class GameObject;

class Transform : public Component
{
public:

    Transform(GameObject* owner);

    virtual ~Transform() override;

    void CleanUp() override;
    
    ComponentType GetType() override { return ComponentType::Transform; };
    bool IsType(ComponentType type) override { return type == ComponentType::Transform; };
    bool IsIncompatible(ComponentType type) override { return type == ComponentType::Transform; };

    void Save(Config& componentNode) override;
    void Load(Config& componentNode) override;

    void SetLocalPosition(const glm::vec3& _position);
    void SetGlobalPosition(const glm::vec3& pos);
    void SetLocalEulerRotation(const glm::vec3& _rotation);
    void SetLocalQuaternionRotation(const glm::quat& _rotation);
    void SetGlobalQuaternionRotation(const glm::quat& rot);
    void SetLocalScale(const glm::vec3& scale);
    void SetGlobalScale(const glm::vec3& scale);
    
    void SetLocalTransform(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scl);
    void SetLocalMatrix(const glm::mat4& localNatrix);

    void OnTransformChanged();

    const glm::vec3& GetLocalPosition();
    const glm::vec3 GetGlobalPosition();
    const glm::vec3& GetLocalEulerRotation();
    const glm::quat& GetLocalQuaterionRotation();
    const glm::quat GetGlobalQuaterionRotation();
    const glm::vec3& GetLocalScale();
    const glm::vec3 GetGlobalScale();

    const glm::mat4& GetLocalMatrix();
    const glm::mat4& GetGlobalMatrix();
    void InvalidateGlobalMatrix();

    void OnEditor() override;

public:
    

private:    
    glm::vec3 position;
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 eulerRotation;
    
    glm::mat4 globalMatrix;
    glm::mat4 localMatrix;
    bool dirtyGlobalMatrix;
    bool dirtyLocalMatrix;

};