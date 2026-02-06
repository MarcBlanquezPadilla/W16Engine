#include "Rigidbody.h"
#include "Collider.h"
#include "../Engine.h"
#include "../ModulePhysics.h"
#include "../ModuleEvents.h"
#include "../ModuleTime.h"
#include "../GameObject.h"
#include "Transform.h"
#include "imgui.h"

#include "../utils/Log.h"


Rigidbody::Rigidbody(GameObject* owner) : Component(owner)
{
    name = "Rigidbody";
    CreateBody();
}

Rigidbody::~Rigidbody()
{
    
}

void Rigidbody::FixedUpdate() 
{
    if (Engine::GetInstance().moduleTime->GetIsRunning())
    {
        if (!actor) return;

        if (type == Type::DYNAMIC) {
            physx::PxTransform pose = actor->getGlobalPose();
            owner->transform->SetGlobalPosition(glm::vec3(pose.p.x, pose.p.y, pose.p.z));
            physx::PxQuat q = pose.q;
            owner->transform->SetGlobalQuaternionRotation(glm::quat(q.w, q.x, q.y, q.z));
        }
        else if (type == Type::KINEMATIC) {

            glm::vec3 p = owner->transform->GetGlobalPosition();
            glm::quat q = owner->transform->GetGlobalQuaterionRotation();

            physx::PxTransform targetPose(
                physx::PxVec3(p.x, p.y, p.z),
                physx::PxQuat(q.x, q.y, q.z, q.w)
            );

            physx::PxRigidDynamic* dyn = actor->is<physx::PxRigidDynamic>();
            if (dyn) dyn->setKinematicTarget(targetPose);
        }
    }
}

void Rigidbody::Update() {

    if (!actor || !owner->transform) return;
    
    if (!Engine::GetInstance().moduleTime->GetIsRunning())
    {
        glm::vec3 pos = owner->transform->GetGlobalPosition();
        glm::quat rot = owner->transform->GetGlobalQuaterionRotation();

        physx::PxTransform pose(
            physx::PxVec3(pos.x, pos.y, pos.z),
            physx::PxQuat(rot.x, rot.y, rot.z, rot.w)
        );

        actor->setGlobalPose(pose);
    }
}

void Rigidbody::OnEnable() 
{

}

void Rigidbody::OnDisable() 
{

}

void Rigidbody::CleanUp()
{
    for (Collider* col : attachedColliders) {
        if (col) {
            col->attachedRigidbody = nullptr;
        }
    }
    attachedColliders.clear();

    if (actor) {
        actor->userData = nullptr;
        Engine::GetInstance().modulePhysics->GetScene()->removeActor(*actor);
        actor->release();
        actor = nullptr;
    }
    OnDisable();
}


void Rigidbody::OnEditor()
{
    const char* bodyTypes[] = { "Static", "Dynamic", "Kinematic" };
    int currentType = (int)this->type;
    ImGui::Text("Body Type:");
    ImGui::PushItemWidth(-FLT_MIN);
    if (ImGui::Combo("##Type", &currentType, bodyTypes, IM_ARRAYSIZE(bodyTypes))) {

        SetType((Type)currentType);
    }
    ImGui::PopItemWidth();

    ImGui::Separator();

    if (actor && type != Type::STATIC) {
        physx::PxRigidDynamic* dynamicActor = actor->is<physx::PxRigidDynamic>();
        bool isKinematic = (type == Type::KINEMATIC);

        if (ImGui::BeginTable("RigidbodyTable", 2, ImGuiTableFlags_Resizable)) {
            ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Mass");
            ImGui::TableNextColumn();
            ImGui::PushItemWidth(-FLT_MIN);
            float mass = dynamicActor->getMass();
            if (ImGui::InputFloat("##Mass", &mass)) {
                SetMass(mass);
            }
            ImGui::PopItemWidth();

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Use CCD");
            ImGui::TableNextColumn();
            bool ccd = IsUsingCCD();
            if (ImGui::Checkbox("##CCD", &ccd)) {
                SetUseCCD(ccd);
            }

            ImGui::EndTable();
            ImGui::Separator();

            if (ImGui::BeginTable("ConstraintsTable", 4)) {

                bool freezePX = false;
                bool freezePY = false;
                bool freezePZ = false;
                bool freezeRX = false;
                bool freezeRY = false;
                bool freezeRZ = false;

                bool changed = false;

                GetConstraints(freezePX, freezePY, freezePZ, freezeRX, freezeRY, freezeRZ);

                ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("X", ImGuiTableColumnFlags_WidthFixed, 15.0f);
                ImGui::TableSetupColumn("Y", ImGuiTableColumnFlags_WidthFixed, 15.0f);
                ImGui::TableSetupColumn("Z", ImGuiTableColumnFlags_WidthFixed, 15.0f);

                ImGui::TableNextColumn();
                ImGui::Text("Constraints");
                ImGui::TableNextColumn();
                ImGui::Text("X");
                ImGui::TableNextColumn();
                ImGui::Text("Y");
                ImGui::TableNextColumn();
                ImGui::Text("Z");

                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("Position");
                ImGui::TableNextColumn(); if (ImGui::Checkbox("##FPX", &freezePX)) changed = true;
                ImGui::TableNextColumn(); if (ImGui::Checkbox("##FPY", &freezePY)) changed = true;
                ImGui::TableNextColumn(); if (ImGui::Checkbox("##FPZ", &freezePZ)) changed = true;

                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("Rotation");
                ImGui::TableNextColumn(); if (ImGui::Checkbox("##FRX", &freezeRX)) changed = true;
                ImGui::TableNextColumn(); if (ImGui::Checkbox("##FRY", &freezeRY)) changed = true;
                ImGui::TableNextColumn(); if (ImGui::Checkbox("##FRZ", &freezeRZ)) changed = true;

                if (changed)
                {
                    SetConstraints(freezePX, freezePY, freezePZ, freezeRX, freezeRY, freezeRZ);
                }
            }

            ImGui::Separator();

            if (!isKinematic) {

                // GRAVEDAD
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Use Gravity");
                ImGui::TableNextColumn();
                bool useGravity = !(actor->getActorFlags() & physx::PxActorFlag::eDISABLE_GRAVITY);
                if (ImGui::Checkbox("##Gravity", &useGravity)) {
                    SetUseGravity(useGravity);
                }

                // LINEAR DRAG
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Linear Damping");
                ImGui::TableNextColumn();
                ImGui::PushItemWidth(-FLT_MIN);
                float linDrag = dynamicActor->getLinearDamping();
                if (ImGui::InputFloat("##Linear Damping", &linDrag)) {
                    SetLinearDamping(linDrag);
                }
                ImGui::PopItemWidth();

                // ANGULAR DRAG
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Angular Damping");
                ImGui::TableNextColumn();
                ImGui::PushItemWidth(-FLT_MIN);
                float angDrag = dynamicActor->getAngularDamping();
                if (ImGui::InputFloat("##Angular Damping", &angDrag)) {
                    SetAngularDamping(angDrag);
                }
                ImGui::PopItemWidth();
            }

            ImGui::EndTable();
            ImGui::Separator();
        }
        physx::PxVec3 vel = dynamicActor->getLinearVelocity();
        ImGui::Text("Velocity:");
        ImGui::Text("X:%.2f | Y:%.2f | Z:%.2f", vel.x, vel.y, vel.z);
    }
}

void Rigidbody::CollectColliders(GameObject* obj, std::vector<Collider*>& list) {

    Collider* col = (Collider*)obj->GetComponent(ComponentType::Collider);
    if (col || col->GetEnabled()) list.push_back(col);

    for (GameObject* child : obj->childs) {

        if (child->GetComponent(ComponentType::Rigidbody) == nullptr) {
            CollectColliders(child, list);
        }
    }
}

void Rigidbody::CreateBody()
{
    auto* physicsModule = Engine::GetInstance().modulePhysics;
    auto* physics = physicsModule->GetPhysics();
    auto* trans = owner->transform;

    if (actor) {
        physicsModule->GetScene()->removeActor(*actor);
        actor->release();
        actor = nullptr;
    }

    for (Collider* col : attachedColliders) {
        if (col) col->attachedRigidbody = nullptr;
    }
    attachedColliders.clear();

    physx::PxRigidActor* tempActor = nullptr;

    glm::vec3 pos = trans->GetGlobalPosition();
    glm::quat rot = trans->GetGlobalQuaterionRotation();
    physx::PxTransform pxTransform(
        physx::PxVec3(pos.x, pos.y, pos.z),
        physx::PxQuat(rot.x, rot.y, rot.z, rot.w)
    );

    if (type == Type::STATIC) {
        tempActor = physics->createRigidStatic(pxTransform);
    }
    else {
        tempActor = physics->createRigidDynamic(pxTransform);
        if (type == Type::KINEMATIC)
            tempActor->is<physx::PxRigidDynamic>()->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
    }
    tempActor->userData = (void*)this;
    CollectListeners();
    physicsModule->GetScene()->addActor(*tempActor);

    std::vector<Collider*> colliders;
    CollectColliders(owner, colliders);

    for (Collider* col : colliders) {
        
        AttachCollider(col);
        physx::PxGeometry* geo = col->GetGeometry();

        float sF, dF, rest;
        col->GetMaterialValues(sF, dF, rest);
        physx::PxMaterial* mat = physics->createMaterial(sF, dF, rest);

        glm::quat relRot = glm::inverse(trans->GetGlobalQuaterionRotation()) * col->owner->transform->GetGlobalQuaterionRotation();

        glm::vec3 pivotRelPos = col->owner->transform->GetGlobalPosition() - trans->GetGlobalPosition();

        glm::vec3 scaledCenter = col->GetCenter() * col->owner->transform->GetGlobalScale();
        glm::vec3 rotatedOffset = relRot * scaledCenter;

        glm::vec3 finalPos = pivotRelPos + rotatedOffset;

        physx::PxTransform localPose(
            physx::PxVec3(finalPos.x, finalPos.y, finalPos.z),
            physx::PxQuat(relRot.x, relRot.y, relRot.z, relRot.w)
        );

        if (col->IsType(ComponentType::CapsuleCollider)) {
            physx::PxQuat rotateToY = physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1));
            localPose.q *= rotateToY;
        }

        physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*tempActor, *geo, *mat);
        shape->setLocalPose(localPose);

        shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !col->IsTrigger());
        shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, col->IsTrigger());

        mat->release();
        delete geo;
    }

    actor = tempActor;
    SyncPropertiesToPhysics();
}

void Rigidbody::AttachCollider(Collider* collider)
{
    attachedColliders.push_back(collider);
    collider->attachedRigidbody = this;
    if (actor) {
        CreateBody();
    }
}


void Rigidbody::UnattachCollider(Collider* collider)
{
    if (!collider) return;

    auto& list = attachedColliders;
    list.erase(std::remove(list.begin(), list.end(), collider), list.end());

    collider->attachedRigidbody = nullptr;

    if (actor) {
        CreateBody();
    }
}

void Rigidbody::SetType(Type newType) {
   
    if (this->type == newType) return;

    Type oldType = this->type;
    this->type = newType;

    bool needsHardReset = (oldType == Type::STATIC || newType == Type::STATIC);

    if (needsHardReset)
    {
        CreateBody();
    }
    else
    {
        physx::PxRigidDynamic* dyn = actor->is<physx::PxRigidDynamic>();
        if (dyn) {
            bool kinematicEnabled = (newType == Type::KINEMATIC);
            dyn->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, kinematicEnabled);

            if (!kinematicEnabled) {
                WakeUp();
            }
        }
    }
}

void Rigidbody::EnableSimulation(bool enable)
{
    if (actor)
    {
        actor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, !enable);
        if (enable)
        {
            if (auto* dyn = GetDynamic()) 
            {
                WakeUp();
            }
        }  
    }
}

void Rigidbody::Save(Config& config)
{
    config.SetInt("Type", (int)type);
    config.SetFloat("Mass", mass);
    config.SetFloat("LinearDamping", linearDamping);
    config.SetFloat("AngularDamping", angularDamping);
    config.SetBool("UseGravity", useGravity);
    config.SetBool("UseCCD", useContiniusCollisionDetection);
    config.SetBool("FreezePosX", freezePosX);
    config.SetBool("FreezePosY", freezePosY);
    config.SetBool("FreezePosZ", freezePosZ);
    config.SetBool("FreezeRotX", freezeRotX);
    config.SetBool("FreezeRotY", freezeRotY);
    config.SetBool("FreezeRotZ", freezeRotZ);
}

void Rigidbody::Load(Config& config)
{
    type = (Type)config.GetInt("Type");
    mass = config.GetFloat("Mass", 1.0f);
    linearDamping = config.GetFloat("LinearDamping");
    angularDamping = config.GetFloat("AngularDamping");
    useGravity = config.GetBool("UseGravity", true);
    useContiniusCollisionDetection = config.GetBool("UseCCD");
    freezePosX = config.GetBool("FreezePosX");
    freezePosY = config.GetBool("FreezePosY");
    freezePosZ = config.GetBool("FreezePosZ");
    freezeRotX = config.GetBool("FreezeRotX");
    freezeRotY = config.GetBool("FreezeRotY");
    freezeRotZ = config.GetBool("FreezeRotZ");
    CreateBody();
}



void Rigidbody::AddForce(const glm::vec3& force, ForceMode mode) {
    if (type == Type::DYNAMIC && actor) {
        
        physx::PxForceMode::Enum m = physx::PxForceMode::Enum::eFORCE;
        switch (mode)
        {
            case IMPULSE:
                m = physx::PxForceMode::Enum::eIMPULSE;
                break;
            case VELOCITY_CHANGE:
                m = physx::PxForceMode::Enum::eVELOCITY_CHANGE;
                break;
            case FORCE:
                m = physx::PxForceMode::Enum::eFORCE;
                break;
            case ACCELERATION:
                m = physx::PxForceMode::Enum::eACCELERATION;
                break;
        }

        if (auto* dyn = GetDynamic()) 
        {
            WakeUp();
            dyn->addForce(physx::PxVec3(force.x, force.y, force.z), m);
        }
    }
}

void Rigidbody::AddTorque(const glm::vec3& force, ForceMode mode) {
    if (type == Type::DYNAMIC && actor) {

        physx::PxForceMode::Enum m = physx::PxForceMode::Enum::eFORCE;
        switch (mode)
        {
        case IMPULSE:
            m = physx::PxForceMode::Enum::eIMPULSE;
            break;
        case VELOCITY_CHANGE:
            m = physx::PxForceMode::Enum::eVELOCITY_CHANGE;
            break;
        case FORCE:
            m = physx::PxForceMode::Enum::eFORCE;
            break;
        case ACCELERATION:
            m = physx::PxForceMode::Enum::eACCELERATION;
            break;
        }

        if (auto* dyn = GetDynamic()) 
        {
            WakeUp();
            dyn->addTorque(physx::PxVec3(force.x, force.y, force.z), m);
        }
    }
}

void Rigidbody::SetLinearVelocity(const glm::vec3& velocity) {
    
    if (auto* dyn = GetDynamic()) dyn->setLinearVelocity(physx::PxVec3(velocity.x, velocity.y, velocity.z));
}

glm::vec3 Rigidbody::GetLinearVelocity() const
{
    if (actor && type != Type::STATIC)
    {
        physx::PxRigidBody* body = actor->is<physx::PxRigidBody>();
        if (body)
        {
            physx::PxVec3 v = body->getLinearVelocity();

            return glm::vec3(v.x, v.y, v.z);
        }
    }

    return glm::vec3(0.0f, 0.0f, 0.0f);
}

void Rigidbody::WakeUp()
{
    if (type != DYNAMIC) return;

    if (auto* dyn = GetDynamic())
    {
        bool inScene = (dyn->getScene() != nullptr);
        bool simEnabled = !(dyn->getActorFlags() & physx::PxActorFlag::eDISABLE_SIMULATION);

        if (inScene && simEnabled)
        {
            dyn->wakeUp();
        }
    }
}

void Rigidbody::PutToSleep()
{
    if (type != DYNAMIC) return;

    if (auto* dyn = GetDynamic())
    {
        bool inScene = (dyn->getScene() != nullptr);
        bool simEnabled = !(dyn->getActorFlags() & physx::PxActorFlag::eDISABLE_SIMULATION);

        if (inScene && simEnabled)
        {
            dyn->putToSleep();
        }
    }
}

bool Rigidbody::IsSleeping()
{
    if (type == DYNAMIC)
    {
        if (auto* dyn = GetDynamic())
        {
            return dyn->isSleeping();
        }
    }

    return false;
}

void Rigidbody::SyncPropertiesToPhysics() {
    if (!actor) return;

    actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !useGravity);

    physx::PxRigidDynamic* dyn = GetDynamic();
    if (dyn) {
        dyn->setMass(mass);
        dyn->setLinearDamping(linearDamping);
        dyn->setAngularDamping(angularDamping);

        dyn->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, useContiniusCollisionDetection);

        dyn->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X, freezePosX);
        dyn->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, freezePosY);
        dyn->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, freezePosZ);
        dyn->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, freezeRotX);
        dyn->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, freezeRotY);
        dyn->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, freezeRotZ);

        WakeUp();
    }
}

void Rigidbody::FreezePosition(bool x, bool y, bool z)
{
    freezePosX = x;
    freezePosY = y;
    freezePosZ = z;
    SyncPropertiesToPhysics();
}

void Rigidbody::FreezeRotation(bool x, bool y, bool z)
{
    freezeRotX = x;
    freezeRotY = y;
    freezeRotZ = z;
    SyncPropertiesToPhysics();
}

void Rigidbody::SetConstraints(bool moveX, bool moveY, bool moveZ, bool rotateX, bool rotateY, bool rotateZ) 
{
    freezePosX = moveX;
    freezePosY = moveY;
    freezePosZ = moveZ;

    freezeRotX = rotateX;
    freezeRotY = rotateY;
    freezeRotZ = rotateZ;

    SyncPropertiesToPhysics();
}

void Rigidbody::GetConstraints(bool& moveX, bool& moveY, bool& moveZ, bool& rotateX, bool& rotateY, bool& rotateZ)
{
    moveX = freezePosX;
    moveY = freezePosY;
    moveZ = freezePosZ;
    rotateX = freezeRotX;
    rotateY = freezeRotY;
    rotateZ = freezeRotZ;
}

void Rigidbody::SetMass(float newMass) 
{
    mass = newMass;
    SyncPropertiesToPhysics();
}

void Rigidbody::SetLinearDamping(float damping) 
{
    linearDamping = damping;
    SyncPropertiesToPhysics();
}

void Rigidbody::SetAngularDamping(float damping) 
{
    angularDamping = damping;
    SyncPropertiesToPhysics();
}

void Rigidbody::SetUseGravity(bool use) 
{
    useGravity = use;
    SyncPropertiesToPhysics();
}

void Rigidbody::SetUseCCD(bool enable) 
{
    useContiniusCollisionDetection = enable;
    SyncPropertiesToPhysics();
}

void Rigidbody::CastPhysicsEvent(PhysicsEventType type, Rigidbody* other)
{
    for (PhysicsEventsListener* listener : listeners)
    {
        switch (type)
        {
            case PhysicsEventType::ON_COLLISION_ENTER: listener->OnCollisionEnter(other); break;
            case PhysicsEventType::ON_COLLISION_STAY:  listener->OnCollisionStay(other);  break;
            case PhysicsEventType::ON_COLLISION_EXIT:  listener->OnCollisionExit(other);  break;

            case PhysicsEventType::ON_TRIGGER_ENTER:   listener->OnTriggerEnter(other);   break;
            case PhysicsEventType::ON_TRIGGER_STAY:    listener->OnTriggerStay(other);    break;
            case PhysicsEventType::ON_TRIGGER_EXIT:    listener->OnTriggerExit(other);    break;
        }
    }
}

void Rigidbody::CollectListeners()
{
    listeners.clear();

    for (auto const& pair : owner->components)
    {
        Component* comp = pair.second;

        PhysicsEventsListener* listener = dynamic_cast<PhysicsEventsListener*>(comp);

        if (listener)
        {
            listeners.push_back(listener);
        }
    }
}

void Rigidbody::OnComponentAdded(Component* component)
{
    if (dynamic_cast<PhysicsEventsListener*>(component))
    {
        CollectListeners();
    }
}

void Rigidbody::OnComponentRemoved(Component* component)
{
    if (dynamic_cast<PhysicsEventsListener*>(component))
    {
        CollectListeners();
    }
}