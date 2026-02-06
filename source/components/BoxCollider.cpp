#include "../Engine.h"
#include "../ModuleRender.h"
#include "../GameObject.h"

#include "BoxCollider.h"
#include "Transform.h"
#include "Rigidbody.h"

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

void BoxCollider::Update()
{
    DebugShape();
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


void BoxCollider::DebugShape() 
{
    physx::PxRigidActor* actor = (attachedRigidbody) ? attachedRigidbody->GetActor() : nullptr;
    
    glm::vec3 scale;
    glm::vec3 halfSize;

    glm::vec3 pos;
    glm::quat rot;
    
    glm::vec4 color;

    if (actor)
    {
        physx::PxTransform pose = actor->getGlobalPose();

        scale = owner->transform->GetGlobalScale();
        halfSize = (size * scale) * 0.5f;

        pos = glm::vec3(pose.p.x, pose.p.y, pose.p.z);
        rot = glm::quat(pose.q.w, pose.q.x, pose.q.y, pose.q.z);
        color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
    }
    else
    {
        scale = owner->transform->GetGlobalScale();
        halfSize = (size * scale) * 0.5f;

        pos = owner->transform->GetGlobalPosition();
        rot = owner->transform->GetGlobalQuaterionRotation();
        color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    }

    glm::vec3 v[8] = {
        {-1,-1,-1}, { 1,-1,-1}, { 1, 1,-1}, {-1, 1,-1},
        {-1,-1, 1}, { 1,-1, 1}, { 1, 1, 1}, {-1, 1, 1}
    };

    for (int i = 0; i < 8; ++i) {
        v[i] = pos + (rot * (v[i] * halfSize));
    }

    auto* render = Engine::GetInstance().moduleRender;

    for (int i = 0; i < 4; ++i) {
        render->DrawLine(v[i], v[(i + 1) % 4], color);
        render->DrawLine(v[i + 4], v[((i + 1) % 4) + 4], color);
        render->DrawLine(v[i], v[i + 4], color);
    }
}