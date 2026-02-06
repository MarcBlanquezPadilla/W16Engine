#include "Collider.h"
#include "Rigidbody.h"
#include "../Engine.h"
#include "../ModulePhysics.h"
#include "../GameObject.h"
#include "Transform.h"
#include "RigidBody.h"
#include "imgui.h"
#include "glm/glm.hpp"

Collider::Collider(GameObject* owner) : Component(owner)
{
    name = "Collider";
}

Collider::~Collider()
{

}

void Collider::Start() 
{
    Rigidbody* rb = (Rigidbody*)owner->GetComponentInParent(ComponentType::Rigidbody);
    if (rb) rb->CreateBody();
}

void Collider::OnEnable() 
{
    Rigidbody* rb = (Rigidbody*)owner->GetComponentInParent(ComponentType::Rigidbody);
    if (rb) rb->CreateBody();
}

void Collider::OnDisable() 
{
    Rigidbody* rb = (Rigidbody*)owner->GetComponentInParent(ComponentType::Rigidbody);
    if (rb) rb->CreateBody();
}

void Collider::SaveBase(Config& config)
{
    config.SetVector3("Center", center);
    config.SetBool("Trigger", isTrigger);
    config.SetFloat("DynamicFriction", dynamicFriction);
    config.SetFloat("StaticFriction", staticFriction);
    config.SetFloat("Restitution", restitution);
}

void Collider::LoadBase(Config& config)
{
    center = config.GetVector3("Center");
    isTrigger = config.GetBool("Trigger");
    dynamicFriction = config.GetFloat("DynamicFriction");
    staticFriction = config.GetFloat("StaticFriction");
    restitution = config.GetFloat("Restitution");
}

void Collider::OnEditorBase()
{
    //ATRIBUTES
    ImGui::Text("Trigger");
    bool trigger = isTrigger;
    if (ImGui::Checkbox("##Trigger", &trigger))
    {
        SetTrigger(trigger);
    }

    ImGui::Text("Center");
    glm::vec3 center = this->center;
    if (ImGui::InputFloat3("##Center", &center.x))
    {
        SetCenter(center);
    }

    ImGui::Text("Static Friction");
    float sF = staticFriction;
    if (ImGui::InputFloat("##Static Friction", &sF))
    {
        SetStaticFriction(sF);
    }

    ImGui::Text("Dynamic Friction");
    float dF = dynamicFriction;
    if (ImGui::InputFloat("##Dynamic Friction", &dF))
    {
        SetDynamicFriction(dF);
    }

    ImGui::Text("Restitution");
    float restitution = this->restitution;
    if (ImGui::InputFloat("##Restitution", &restitution))
    {
        SetRestitution(restitution);
    }
}

void Collider::SetCenter(glm::vec3 center)
{
    this->center = center;
    Rigidbody* rb = (Rigidbody*)owner->GetComponentInParent(ComponentType::Rigidbody);
    if (rb) rb->CreateBody();
}

void Collider::SetTrigger(bool trigger)
{
    isTrigger = trigger;
    Rigidbody* rb = (Rigidbody*)owner->GetComponentInParent(ComponentType::Rigidbody);
    if (rb) rb->CreateBody();
}

void Collider::SetStaticFriction(float staticFriction)
{
    this->staticFriction = glm::clamp(staticFriction, 0.0f, INFINITY);
    Rigidbody* rb = (Rigidbody*)owner->GetComponentInParent(ComponentType::Rigidbody);
    if (rb) rb->CreateBody();
}

void Collider::SetDynamicFriction(float dynamicFriction)
{
    this->dynamicFriction = glm::clamp(dynamicFriction, 0.0f, INFINITY);
    Rigidbody* rb = (Rigidbody*)owner->GetComponentInParent(ComponentType::Rigidbody);
    if (rb) rb->CreateBody();
}
void Collider::SetRestitution(float restitution)
{
    this->restitution = glm::clamp(restitution, 0.0f, INFINITY);
    Rigidbody* rb = (Rigidbody*)owner->GetComponentInParent(ComponentType::Rigidbody);
    if (rb) rb->CreateBody();
}