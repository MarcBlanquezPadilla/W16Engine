#pragma once
#include "Component.h"
#include <glm/glm.hpp>
#include "glm/gtc/quaternion.hpp"

class GameObject;

class Transform : public Component
{
public:

    Transform(GameObject* owner);

    virtual ~Transform() override;

    void CleanUp() override;
    
    ComponentType GetType() override {
        return ComponentType::Transform;
    };

    void Save(Config& componentNode) override;
    void Load(Config& componentNode) override;

    void SetEulerRotation(glm::vec3 _rotation);
    void SetQuaternionRotation(glm::quat _rotation);
    void SetPosition(glm::vec3 _position);
    void SetScale(glm::vec3 _position);
    
    void SetLocalMatrix(const glm::mat4& localNatrix);

    void OnTransformChanged();

    glm::vec3 GetPosition();
    glm::vec3 GetGlobalPosition();
    glm::vec3 GetEulerRotation();
    glm::quat GetQuaterionRotation();
    glm::vec3 GetScale();

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