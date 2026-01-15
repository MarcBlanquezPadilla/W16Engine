#include "Transform.h"
#include "Component.h"

#include "../ModuleEvents.h"
#include "../Engine.h"
#include "../utils/Log.h"
#include "../GameObject.h"

#include <vector>
#include <assimp/scene.h>
#include "imgui.h"
#include "ImGuizmo.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/matrix_decompose.hpp"

Transform::Transform(GameObject* owner) : Component(owner)
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

void Transform::Save(Config& componentNode)
{
    componentNode.SetVector3("Position", GetLocalPosition());
    componentNode.SetQuat("Rotation", GetLocalQuaterionRotation());
    componentNode.SetVector3("Scale", GetLocalScale());
}

void Transform::Load(Config& componentNode)
{
    SetLocalPosition(componentNode.GetVector3("Position"));
    SetLocalQuaternionRotation(componentNode.GetQuat("Rotation"));
    SetLocalScale(componentNode.GetVector3("Scale"));
}

const glm::mat4& Transform::GetLocalMatrix()
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

const glm::mat4& Transform::GetGlobalMatrix()
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
    if (dirtyGlobalMatrix) return;

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

const glm::vec3& Transform::GetLocalPosition()
{
    return position;
}

const glm::vec3& Transform::GetLocalEulerRotation()
{
    return eulerRotation;
}

const glm::quat& Transform::GetLocalQuaterionRotation()
{
    return rotation;
}

const glm::vec3& Transform::GetLocalScale()
{
    return scale;
}

void Transform::SetLocalPosition(const glm::vec3& _position)
{
    dirtyLocalMatrix = true;
    position = _position;
    OnTransformChanged();
}

void Transform::SetLocalEulerRotation(const glm::vec3& _rotation)
{
    dirtyLocalMatrix = true;
    eulerRotation = _rotation;

    glm::vec3 rotationRadians = glm::radians(_rotation);
    rotation = glm::quat(rotationRadians);
    OnTransformChanged();
}

void Transform::SetLocalQuaternionRotation(const glm::quat& _rotationQuat)
{
    dirtyLocalMatrix = true;
    rotation = _rotationQuat;
    eulerRotation = glm::degrees(glm::eulerAngles(rotation));
    OnTransformChanged();
}

void Transform::SetLocalScale(const glm::vec3& _scale)
{
    dirtyLocalMatrix = true;
    scale = _scale;
    OnTransformChanged();
}

void Transform::OnTransformChanged()
{
    InvalidateGlobalMatrix();
    if (owner->GetStatic()) Engine::GetInstance().moduleEvents->PublishImmediate(Event(Event::Type::StaticTransformChanged, owner));
}

void Transform::OnEditor()
{
    if (ImGui::CollapsingHeader("Transform"))
    {
        //ATRIBUTES
        ImGui::Text("Position");
        glm::vec3 current_position = GetLocalPosition();
        if (ImGui::InputFloat3("##Pos", &current_position.x))
        {
            SetLocalPosition(current_position);
        }

        ImGui::Text("Rotation");
        glm::vec3 current_euler_degrees = GetLocalEulerRotation();
        if (ImGui::InputFloat3("##Rot", &current_euler_degrees.x))
        {
            SetLocalEulerRotation(current_euler_degrees);
        }

        ImGui::Text("Scale");
        glm::vec3 current_scale = GetLocalScale();
        if (ImGui::InputFloat3("##Scale", &current_scale.x))
        {
            SetLocalScale(current_scale);
        }
    }
}

void Transform::SetLocalMatrix(const glm::mat4& newLocalMatrix)
{
    localMatrix = newLocalMatrix;
    dirtyLocalMatrix = false;

    glm::vec3 skew;
    glm::vec4 perspective;
    glm::quat rotationQuat;

    if (glm::decompose(newLocalMatrix, scale, rotationQuat, position, skew, perspective))
    {
        rotation = rotationQuat;

        eulerRotation = glm::degrees(glm::eulerAngles(rotationQuat));
    }

    InvalidateGlobalMatrix();
}

const glm::vec3& Transform::GetGlobalPosition()
{
    glm::mat4 globalMatrix = GetGlobalMatrix();

    return glm::vec3(globalMatrix[3]);
}

const glm::quat& Transform::GetGlobalQuaterionRotation()
{
    glm::mat4 globalMat = GetGlobalMatrix();

    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;

    glm::decompose(globalMat, scale, rotation, translation, skew, perspective);

    return rotation;
}

const glm::vec3& Transform::GetGlobalScale()
{
    glm::mat4 globalMat = GetGlobalMatrix();

    glm::vec3 globalScale;
    globalScale.x = glm::length(glm::vec3(globalMat[0]));
    globalScale.y = glm::length(glm::vec3(globalMat[1]));
    globalScale.z = glm::length(glm::vec3(globalMat[2]));

    return globalScale;
}

void Transform::SetGlobalPosition(const glm::vec3& targetPos)
{
    if (owner->parent == nullptr)
    {
        SetLocalPosition(targetPos);
        return;
    }

    glm::mat4 parentGlobal = owner->parent->transform->GetGlobalMatrix();
    glm::mat4 parentInverse = glm::inverse(parentGlobal);

    glm::vec4 localPos4 = parentInverse * glm::vec4(targetPos, 1.0f);

    SetLocalPosition(glm::vec3(localPos4));
}

void Transform::SetGlobalQuaternionRotation(const glm::quat& targetRot)
{
    if (owner->parent == nullptr)
    {
        SetLocalQuaternionRotation(targetRot);
        return;
    }

    glm::quat parentGlobalRot = owner->parent->transform->GetGlobalQuaterionRotation();
    glm::quat parentInverse = glm::inverse(parentGlobalRot);

    glm::quat localRot = parentInverse * targetRot;

    SetLocalQuaternionRotation(localRot);
}

void Transform::SetGlobalScale(const glm::vec3& targetScale)
{
    if (owner->parent == nullptr)
    {
        SetLocalScale(targetScale);
        return;
    }

    glm::vec3 parentGlobalScale = owner->parent->transform->GetGlobalScale();

    glm::vec3 newLocalScale = targetScale;
    if (parentGlobalScale.x != 0) newLocalScale.x /= parentGlobalScale.x;
    if (parentGlobalScale.y != 0) newLocalScale.y /= parentGlobalScale.y;
    if (parentGlobalScale.z != 0) newLocalScale.z /= parentGlobalScale.z;

    SetLocalScale(newLocalScale);
}