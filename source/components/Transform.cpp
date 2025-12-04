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

void Transform::Save(Config componentNode)
{
    componentNode.SetVector3("Position", GetPosition());
    componentNode.SetQuat("Rotation", GetQuaterionRotation());
    componentNode.SetVector3("Scale", GetScale());
}

void Transform::Load(Config componentNode)
{
    SetPosition(componentNode.GetVector3("Position"));
    SetQuaternionRotation(componentNode.GetQuat("Rotation"));
    SetScale(componentNode.GetVector3("Scale"));
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
    eulerRotation = glm::degrees(glm::eulerAngles(rotation));
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
    Engine::GetInstance().moduleEvents->PublishImmediate(Event(Event::Type::TransformChanged, owner));
}

void Transform::OnEditor()
{
    if (ImGui::CollapsingHeader("Transform"))
    {
        //ATRIBUTES
        ImGui::Text("Position");
        glm::vec3 current_position = GetPosition();
        if (ImGui::InputFloat3("##Pos", &current_position.x))
        {
            SetPosition(current_position);
        }

        ImGui::Text("Rotation");
        glm::vec3 current_euler_degrees = GetEulerRotation();
        if (ImGui::InputFloat3("##Rot", &current_euler_degrees.x))
        {
            SetEulerRotation(current_euler_degrees);
        }

        ImGui::Text("Scale");
        glm::vec3 current_scale = GetScale();
        if (ImGui::InputFloat3("##Scale", &current_scale.x))
        {
            SetScale(current_scale);
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

glm::vec3 Transform::GetGlobalPosition()
{
    glm::mat4 globalMatrix = GetGlobalMatrix();

    return glm::vec3(globalMatrix[3]);
}