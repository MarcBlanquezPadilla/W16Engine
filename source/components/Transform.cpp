#include "Transform.h"
#include "Component.h"

#include "../EventSystem.h"
#include "../Engine.h"
#include "../utils/Log.h"
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

void Transform::CleanUp()
{

}

void Transform::Save(pugi::xml_node componentNode)
{
    componentNode.append_attribute("type") = (int)GetType();

    pugi::xml_node positionNode = componentNode.append_child("Position");
    positionNode.append_attribute("x") = GetPosition().x;
    positionNode.append_attribute("y") = GetPosition().y;
    positionNode.append_attribute("z") = GetPosition().z;

    pugi::xml_node rotationNode = componentNode.append_child("Rotation");
    rotationNode.append_attribute("x") = GetQuaterionRotation().x;
    rotationNode.append_attribute("y") = GetQuaterionRotation().y;
    rotationNode.append_attribute("z") = GetQuaterionRotation().z;
    rotationNode.append_attribute("w") = GetQuaterionRotation().w;

    pugi::xml_node scaleNode = componentNode.append_child("Scale");
    scaleNode.append_attribute("x") = GetScale().x;
    scaleNode.append_attribute("y") = GetScale().y;
    scaleNode.append_attribute("z") = GetScale().z;
}

void Transform::Load(pugi::xml_node componentNode)
{
    pugi::xml_node positionNode = componentNode.child("Position");
    if (positionNode)
    {
        glm::vec3 position = glm::vec3(
            positionNode.attribute("x").as_float(),
            positionNode.attribute("y").as_float(),
            positionNode.attribute("z").as_float()
        );

        SetPosition(position);
    }
    else
    {
        LOG("Error: <Position> node not found for this component.");
    }

    pugi::xml_node rotationNode = componentNode.child("Rotation");
    if (rotationNode)
    {
        glm::quat rotation = glm::quat(
            rotationNode.attribute("w").as_float(),
            rotationNode.attribute("x").as_float(),
            rotationNode.attribute("y").as_float(),
            rotationNode.attribute("z").as_float()
        );

        SetQuaternionRotation(rotation);
    }
    else
    {
        LOG("Error: <Rotation> node not found for this component.");
    }

    pugi::xml_node scaleNode = componentNode.child("Scale");
    if (scaleNode)
    {
        glm::vec3 scale = glm::vec3(
            scaleNode.attribute("x").as_float(),
            scaleNode.attribute("y").as_float(),
            scaleNode.attribute("z").as_float()
        );

        SetScale(scale);
    }
    else
    {
        LOG("Error: <Scale> node not found for this component.");
    }
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
    dirtyLocalMatrix = true;
    position = _position;
    OnTransformChanged();
}

void Transform::SetEulerRotation(glm::vec3 _rotation)
{
    dirtyLocalMatrix = true;
    eulerRotation = _rotation;

    glm::vec3 rotationRadians = glm::radians(_rotation);
    rotation = glm::quat(rotationRadians);
    OnTransformChanged();
}

void Transform::SetQuaternionRotation(glm::quat _rotationQuat)
{
    dirtyLocalMatrix = true;
    rotation = _rotationQuat;
    OnTransformChanged();
}

void Transform::SetScale(glm::vec3 _scale)
{
    dirtyLocalMatrix = true;
    scale = _scale;
    OnTransformChanged();
}

void Transform::OnTransformChanged()
{
    InvalidateGlobalMatrix();
    Engine::GetInstance().events->PublishImmediate(TransformChangedEvent(owner, GetPosition(), GetEulerRotation(), GetScale()));
}




