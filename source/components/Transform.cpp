#include "Transform.h"
#include "../Log.h"
#include "Component.h"
#include "../GameObject.h"
#include <vector>
#include <assimp/scene.h>

Transform::Transform(GameObject* owner, bool enabled) : Component(owner, enabled)
{
    position = glm::vec3(0.0f, 0.0f, 0.0f);
    scale = glm::vec3(1.0f, 1.0f, 1.0f);
    rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    eulerRotation = glm::vec3(0.0f, 0.0f, 0.0f);
}

Transform::~Transform()
{
    
}

glm::mat4 Transform::GetLocalMatrix() const
{
    glm::mat4 matTranslation = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 matRotation = glm::mat4_cast(rotation);
    glm::mat4 matScale = glm::scale(glm::mat4(1.0f), scale);

    return matTranslation * matRotation * matScale;
}

glm::mat4 Transform::GetGlobalMatrix() const
{
    if (dirtyMatrix)
    {
        glm::mat4 matTranslation = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 matRotation = glm::mat4_cast(rotation);
        glm::mat4 matScale = glm::scale(glm::mat4(1.0f), scale);

        glm::mat4 localMatrix = matTranslation * matRotation * matScale;
        glm::mat4 con = ((Transform*)owner->GetComponent(ComponentType::Transform))->GetGlobalMatrix() * localMatrix;
        modelMatrix;
    }
    
    return modelMatrix;
}

glm::vec3 Transform::GetPosition()
{
    return position;
}

glm::vec3 Transform::GetEulerRotation()
{
    return eulerRotation;
}

glm::quat Transform::GetQuaterionRotation()
{
    return rotation;
}

glm::vec3 Transform::GetScale()
{
    return scale;
}

void Transform::SetPosition(glm::vec3 _position)
{
    position = _position;
}

void Transform::SetEulerRotation(glm::vec3 _rotation)
{
    eulerRotation = _rotation;

    glm::vec3 rotationRadians = glm::radians(_rotation);
    rotation = glm::quat(rotationRadians);
}

void Transform::SetQuaternionRotation(glm::quat _rotationQuat)
{
    rotation = _rotationQuat;
}

void Transform::SetScale(glm::vec3 _scale)
{
    scale = _scale;
}




