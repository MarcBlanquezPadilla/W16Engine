#include "BoxCollider.h"
#include "Transform.h"
#include "Rigidbody.h"
#include "../GameObject.h"
#include "imgui.h"

BoxCollider::BoxCollider(GameObject* owner) : Collider(owner)
{
    name = "Box Collider";
}

physx::PxGeometry* BoxCollider::GetGeometry()
{
    glm::vec3 scale = owner->transform->GetGlobalScale();
    return new physx::PxBoxGeometry((size.x * scale.x) * 0.5f,
        (size.y * scale.y) * 0.5f,
        (size.z * scale.z) * 0.5f);
}

void BoxCollider::OnEditor()
{
    OnEditorBase();
    ImGui::Separator();

    ImGui::Text("Size");
    glm::vec3 s = size;
    if (ImGui::InputFloat3("##Size", &s.x))
    {
        SetSize(s);
    }
}

void BoxCollider::Save(Config& config)
{
    SaveBase(config);
    config.SetVector3("Size", size);
}

void BoxCollider::Load(Config& config)
{
    LoadBase(config);
    size = config.GetVector3("Size", glm::vec3(1.0f, 1.0f, 1.0f));
    Rigidbody* rb = (Rigidbody*)owner->GetComponentInParent(ComponentType::Rigidbody);
    if (rb) rb->CreateBody();
}

void BoxCollider::SetSize(glm::vec3 size)
{
    this->size.x = glm::clamp(size.x, 0.001f, INFINITY);
    this->size.y = glm::clamp(size.y, 0.001f, INFINITY);
    this->size.z = glm::clamp(size.z, 0.001f, INFINITY);
    Rigidbody* rb = (Rigidbody*)owner->GetComponentInParent(ComponentType::Rigidbody);
    if (rb) rb->CreateBody();
}
