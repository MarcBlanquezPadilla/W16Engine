#include "SphereCollider.h"
#include "Transform.h"
#include "Rigidbody.h"
#include "../GameObject.h"
#include "imgui.h"

SphereCollider::SphereCollider(GameObject* owner) : Collider(owner)
{
    name = "Sphere Collider";
}

physx::PxGeometry* SphereCollider::GetGeometry()
{
    glm::vec3 scale = owner->transform->GetGlobalScale();

    float maxScale = glm::max(scale.x, glm::max(scale.y, scale.z));

    return new physx::PxSphereGeometry(radius * maxScale);
}

void SphereCollider::OnEditor()
{
    OnEditorBase();

    ImGui::Separator();

    ImGui::Text("Radius");
    float r = radius;
    if (ImGui::InputFloat("##Radius", &r))
    {
        SetRadius(r);
    }
}

void SphereCollider::Save(Config& config)
{
    SaveBase(config);
    config.SetFloat("Radius", radius);
}

void SphereCollider::Load(Config& config)
{
    LoadBase(config);
    SetRadius(config.GetFloat("Radius"));
    
}

void SphereCollider::SetRadius(float radius)
{
    this->radius = glm::clamp(radius, 0.0001f, INFINITY);
    Rigidbody* rb = (Rigidbody*)owner->GetComponentInParent(ComponentType::Rigidbody);
    if (rb) rb->CreateBody();
}