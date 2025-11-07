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
    void SetQuaternionRotation(glm::quat _rotation);
    void SetPosition(glm::vec3 _position);
    void SetScale(glm::vec3 _position);
   
    glm::vec3 GetPosition();
    glm::vec3 GetEulerRotation();
    glm::quat GetQuaterionRotation();
    glm::vec3 GetScale();

    glm::mat4 GetLocalMatrix() const;
    glm::mat4 GetGlobalMatrix() const;

public:
    

private:
    glm::vec3 position;
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 eulerRotation;
    
    glm::mat4 modelMatrix;
    bool dirtyMatrix;
};