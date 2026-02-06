#include "CapsuleCollider.h"
#include "Transform.h"
#include "Rigidbody.h"
#include "../GameObject.h"
#include "imgui.h"
#include "glm/glm.hpp"

CapsuleCollider::CapsuleCollider(GameObject* owner) : Collider(owner)
{
    name = "Capsule Collider";
}

physx::PxGeometry* CapsuleCollider::GetGeometry() {
    
    glm::vec3 scale = owner->transform->GetGlobalScale();
   
    float s = glm::max(scale.x, scale.z);
    
    return new physx::PxCapsuleGeometry(radius * s, (height * scale.y) * 0.5f);
}

void CapsuleCollider::OnEditor()
{
    OnEditorBase();

    ImGui::Separator();

    ImGui::Text("Radius");
    float r = radius;
    if (ImGui::InputFloat("##Radius", &r))
    {
        SetRadius(r);
    }

    ImGui::Text("Height");
    float h = height;
    if (ImGui::InputFloat("##Height", &h))
    {
        SetHeight(h);
    }
}

void CapsuleCollider::Save(Config& config)
{
    SaveBase(config);
    config.SetFloat("Radius", radius);
    config.SetFloat("Height", height);
}

void CapsuleCollider::Load(Config& config)
{
    LoadBase(config);
    SetRadius(config.GetFloat("Radius"));
    SetRadius(config.GetFloat("Height"));
}

void CapsuleCollider::SetRadius(float radius)
{
    this->radius = glm::clamp(radius, 0.0001f, INFINITY);
    Rigidbody* rb = (Rigidbody*)owner->GetComponentInParent(ComponentType::Rigidbody);
    if (rb) rb->CreateBody();
}

void CapsuleCollider::SetHeight(float height)
{
    this->height = glm::clamp(height, 0.0001f, INFINITY);
    Rigidbody* rb = (Rigidbody*)owner->GetComponentInParent(ComponentType::Rigidbody);
    if (rb) rb->CreateBody();
}