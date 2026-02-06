#include "ModulePhysics.h"
#include "utils/Log.h"
#include "utils/Time.h"
#include "glm/glm.hpp"
#include "Engine.h"
#include "ModuleRender.h"
#include "ModuleTime.h"

using namespace physx;

ModulePhysics::ModulePhysics(bool startEnabled) : Module(startEnabled) {
    name = "Physics";
}

ModulePhysics::~ModulePhysics() {}

bool ModulePhysics::Awake() {
    
    LOG(LogType::LOG_INFO, "Iniciando PhysX 5.5...");

    gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
    if (!gFoundation) {
        LOG(LogType::LOG_ERROR, "¡Fallo al crear PxFoundation!");
        return false;
    }

    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true);
    if (!gPhysics) {
        LOG(LogType::LOG_ERROR, "¡Fallo al crear PxPhysics!");
        return false;
    }

    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

    gDispatcher = PxDefaultCpuDispatcherCreate(2);

    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;

    gScene = gPhysics->createScene(sceneDesc);

    if (!gScene) {
        LOG(LogType::LOG_ERROR, "¡Fallo al crear PxScene!");
        return false;
    }

    return true;
}

bool ModulePhysics::PreUpdate() {
    
    float dt = Time::realDeltaTime;
    accumulator += dt;

    while (accumulator >= stepSize)
    {
        gScene->simulate(stepSize);

        gScene->fetchResults(true);

        Engine::GetInstance().FixedUpdate();

        accumulator -= stepSize;
    }

    return true;
}

bool ModulePhysics::Update()
{
    if (debugPhysics) {
        gScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.0f);
        gScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
    }
    else {
        gScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 0.0f);
    }

    if (debugPhysics) {
        DrawDebug();
    }
    return true;
}

bool ModulePhysics::CleanUp() {
    
    LOG(LogType::LOG_INFO, "Cleaning PhysX...");

    if (gScene) gScene->release();
    if (gDispatcher) gDispatcher->release();
    if (gPhysics) gPhysics->release();
    if (gFoundation) gFoundation->release();

    return true;
}

void ModulePhysics::DrawDebug()
{
    const PxRenderBuffer& rb = gScene->getRenderBuffer();

    for (PxU32 i = 0; i < rb.getNbLines(); i++)
    {
        const PxDebugLine& line = rb.getLines()[i];

        glm::vec3 start(line.pos0.x, line.pos0.y, line.pos0.z);
        glm::vec3 end(line.pos1.x, line.pos1.y, line.pos1.z);

        glm::vec4 color(1.0f, 1.0f, 0.0f, 1.0f);

        Engine::GetInstance().moduleRender->DrawLine(start, end, color);
    }
}