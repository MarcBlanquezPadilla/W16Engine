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

    dirtyLocalMatrix = true;
    dirtyGlobalMatrix = true;
}

Transform::~Transform()
{
    
}

glm::mat4 Transform::GetLocalMatrix()
{
    if (dirtyLocalMatrix)
    {
        glm::mat4 matTranslation = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 matRotation = glm::mat4_cast(rotation);
        glm::mat4 matScale = glm::scale(glm::mat4(1.0f), scale);

        localMatrix = matTranslation * matRotation * matScale;
        dirtyLocalMatrix = false;
    }

    return localMatrix;
}

glm::mat4 Transform::GetGlobalMatrix()
{
    if (dirtyGlobalMatrix)
    {
        if (owner->parent != nullptr) globalMatrix = owner->parent->transform->GetGlobalMatrix() * GetLocalMatrix();
        else globalMatrix = GetLocalMatrix();
        dirtyGlobalMatrix = false;
    }
    
    return globalMatrix;
}

void Transform::InvalidateGlobalMatrix()
{
    dirtyGlobalMatrix = true;

    if (owner != nullptr)
    {
        for (GameObject* child : owner->childs)
        {
            if (child != nullptr && child->transform != nullptr)
            {
                child->transform->InvalidateGlobalMatrix();
            }
        }
    }
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
    InvalidateGlobalMatrix();
    dirtyLocalMatrix = true;
    position = _position;
}

void Transform::SetEulerRotation(glm::vec3 _rotation)
{
    InvalidateGlobalMatrix();
    dirtyLocalMatrix = true;
    eulerRotation = _rotation;

    glm::vec3 rotationRadians = glm::radians(_rotation);
    rotation = glm::quat(rotationRadians);
}

void Transform::SetQuaternionRotation(glm::quat _rotationQuat)
{
    InvalidateGlobalMatrix();
    dirtyLocalMatrix = true;
    rotation = _rotationQuat;
}

void Transform::SetScale(glm::vec3 _scale)
{
    InvalidateGlobalMatrix();
    dirtyLocalMatrix = true;
    scale = _scale;
}




