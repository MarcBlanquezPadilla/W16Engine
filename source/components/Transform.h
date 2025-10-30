#pragma once
#include "Component.h"
#include <glm/glm.hpp>
#include "glm/gtc/quaternion.hpp"

class GameObject;

class Transform : public Component
{
public:

    Transform(GameObject* owner, bool enabled);

    virtual ~Transform() override;
    
    ComponentType GetType() override {
        return ComponentType::Transform;
    };

    void SetEulerRotation(glm::vec3 _rotation);
    glm::vec3 GetEulerRotation();

    glm::mat4 GetLocalMatrix() const;

public:
    glm::vec3 position;
    glm::vec3 scale;
    glm::quat rotation;

    glm::vec3 eulerRotation;
};