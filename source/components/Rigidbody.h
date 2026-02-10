#pragma once
#include "Component.h"
#include "PhysicsEventsListener.h"
#include "../ModulePhysics.h"
#include "../EventListener.h"
#include <PxPhysicsAPI.h>

class GameObject;
class Collider;
class Joint;

class Rigidbody : public Component
{
public:

    enum ForceMode
    {
        ACCELERATION,
        FORCE,
        IMPULSE,
        VELOCITY_CHANGE
    };

    enum Type
    {
        STATIC,
        DYNAMIC,
        KINEMATIC
    };

    Rigidbody(GameObject* owner);

    virtual ~Rigidbody() override;

    void OnEnable() override;
    void Update() override;
    void FixedUpdate() override;
    void OnDisable() override;

    void CleanUp() override;
    
    ComponentType GetType() override { return ComponentType::Rigidbody; };
    Type GetBodyType() const { return type; };
    bool IsType(ComponentType type) override { return type == ComponentType::Rigidbody; };
    bool IsIncompatible(ComponentType type) override { return type == ComponentType::Rigidbody; };

    physx::PxRigidActor* GetActor() const { return actor; }
    void CollectColliders(GameObject* obj, std::vector<Collider*>& list);

    void Save(Config& componentNode) override;
    void Load(Config& componentNode) override;

    void OnEditor() override;

    void SetType(Type bodyType);
    
    void CreateBody();
    void UnattachCollider(Collider* collider);
    void AttachCollider(Collider* collider);

    void EnableSimulation(bool enable);

    //FISICS
    void WakeUp();
    void PutToSleep();
    bool IsSleeping();

    //CONSTRAINTS
    void FreezePosition(bool x, bool y, bool z);
    void FreezeRotation(bool x, bool y, bool z);
    void SetConstraints(bool moveX, bool moveY, bool moveZ, bool rotateX, bool rotateY, bool rotateZ);
    void GetConstraints(bool& moveX, bool& moveY, bool& moveZ, bool& rotateX, bool& rotateY, bool& rotateZ);


    void AddForce(const glm::vec3& force, ForceMode mode = FORCE);
    void AddTorque(const glm::vec3& torque, ForceMode mode = FORCE);
    void SetLinearVelocity(const glm::vec3& velocity);
    glm::vec3 GetLinearVelocity() const;
    void MovePosition(const glm::vec3& position);
    void MoveRotation(const glm::quat& rotation);

    const float GetMass() { return mass; }
    const float GetLinearDamping() { return linearDamping; }
    const float GetAngularDamping() { return angularDamping; }
    const bool IsUsingGravity() { return useGravity; }
    const bool IsUsingCCD() { return useContiniusCollisionDetection; }

    void SetMass(float mass);
    void SetLinearDamping(float linearDamping);
    void SetAngularDamping(float angularDamping);
    void SetUseGravity(bool useGravity);
    void SetUseCCD(bool useCCD);

    //COLLISIONS
    void CastPhysicsEvent(PhysicsEventType type, Rigidbody* other);

    //JOINTS
    void RegisterJoint(Joint* joint);
    void UnregisterJoint(Joint* joint);

private:    

    void SyncToTransform();
    void SyncPropertiesToPhysics();

    void UpdateShapesGeometry();
    void UpdateShapeLocalPose(physx::PxShape* shape, Collider* col);

    physx::PxRigidDynamic* GetDynamic() { return actor ? actor->is<physx::PxRigidDynamic>() : nullptr; }
    void CollectListeners();

    //EVENTS
    void OnGameObjectEvent(GameObjectEvent event, Component* component);

private: 

    //PROPERTIES
    Type type = STATIC;
    float mass = 1.0f;
    float linearDamping = 0.0f;
    float angularDamping = 0.0f;
    bool useGravity = true;
    bool useContiniusCollisionDetection = false;

    bool freezePosX = false, freezePosY = false, freezePosZ = false;
    bool freezeRotX = false, freezeRotY = false, freezeRotZ = false;

    //KINEMATIC
    glm::vec3 kinematicTargetPos;
    glm::quat kinematicTargetRot;
    bool hasKinematicTarget = false;
    bool isSyncingFromPhysics = false;

    //INTERPOLATION
    physx::PxTransform lastPose;
    physx::PxTransform currentPose;
    float accumulator = 0.0f;

    //POINTERS
    physx::PxRigidActor* actor = nullptr;
    std::vector <PhysicsEventsListener*> listeners;
    std::vector <Collider*> attachedColliders;
    std::vector<Joint*> connectedJoints;
};