#pragma once
#include "Module.h"
#include <PxPhysicsAPI.h>

class ModulePhysics : public Module
{
public:
    ModulePhysics(bool startEnabled = true);
    ~ModulePhysics();

    bool Awake() override;
    bool PreUpdate() override;
    bool Update() override;
    bool CleanUp() override;

    void DrawDebug();

    physx::PxPhysics* GetPhysics() { return gPhysics; }
    physx::PxScene* GetScene() { return gScene; }
    physx::PxMaterial* GetDefaultMaterial() { return gMaterial; }

private:
    
    physx::PxDefaultAllocator gAllocator;
    physx::PxDefaultErrorCallback  gErrorCallback;
    physx::PxFoundation* gFoundation = nullptr;
    physx::PxPhysics* gPhysics = nullptr;
    physx::PxDefaultCpuDispatcher* gDispatcher = nullptr;
    physx::PxScene* gScene = nullptr;
    physx::PxMaterial* gMaterial = nullptr;

    float accumulator = 0.0f;
    float stepSize = 1.0f / 60.0f;

    bool debugPhysics = true;
};