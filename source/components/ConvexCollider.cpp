#include "ConvexCollider.h"
#include "MeshRenderer.h"
#include "../resources/ResourceMesh.h"
#include "../GameObject.h"
#include "../Engine.h"
#include "../ModuleRender.h"
#include "Transform.h"
#include "Rigidbody.h"
#include "imgui.h"
#include "../utils/PhysicsCooker.h"

ConvexCollider::ConvexCollider(GameObject* owner) : Collider(owner) {
    
    name = "Convex Collider";
    cookedMesh = nullptr;
    CookMesh();
}

ConvexCollider::~ConvexCollider() {
    
    if (cookedMesh) {
        cookedMesh->release();
        cookedMesh = nullptr;
    }
}

void ConvexCollider::Update()
{
    DebugShape();
}


physx::PxGeometry* ConvexCollider::GetGeometry() {
    
    if (!cookedMesh) CookMesh();
    if (!cookedMesh) return nullptr;

    glm::vec3 scale = owner->transform->GetGlobalScale();
    return new physx::PxConvexMeshGeometry(cookedMesh, physx::PxMeshScale(physx::PxVec3(scale.x, scale.y, scale.z)));
}

void ConvexCollider::CookMesh() {
    
    MeshRenderer* meshRenderer = (MeshRenderer*)owner->GetComponent(ComponentType::MeshRenderer);

    bool hasValidMesh = meshRenderer && meshRenderer->GetMeshResource() && !meshRenderer->GetMeshResource()->vertices.empty();

    if (cookedMesh) {
        cookedMesh->release();
        cookedMesh = nullptr;
    }

    if (!hasValidMesh)
    {
        if (!meshRenderer) LOG(LogType::LOG_WARNING, "Convex Collider on '%s' ignored: No MeshRenderer component found.", owner->name.c_str());
    }
    else
    {
        auto resource = meshRenderer->GetMeshResource();
        int size = resource->vertices.size();
        std::vector<glm::vec3> vertices(size);

        for (int i = 0; i < size; i++) {
            vertices[i] = resource->vertices[i].position;
        }

        cookedMesh = PhysicsCooker::CookConvex((const float*)vertices.data(), (uint32_t)size, sizeof(glm::vec3));
    }

    if (attachedRigidbody) attachedRigidbody->UpdateShapesGeometry();
}

void ConvexCollider::OnEditor() {
    OnEditorBase();
    ImGui::Separator();

    if (cookedMesh) {
        ImGui::Text("Cooked Mesh Stats:");
        ImGui::BulletText("Vertices: %d", cookedMesh->getNbVertices());
        ImGui::BulletText("Polygons: %d", cookedMesh->getNbPolygons());
    }
    else {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "No mesh cooked!");
    }

    if (ImGui::Button("Recook Convex Mesh")) {
        CookMesh();
        Rigidbody* rb = (Rigidbody*)owner->GetComponentInParent(ComponentType::Rigidbody);
        if (rb) rb->CreateBody();
    }
}

void ConvexCollider::Save(Config& config)
{

}
void ConvexCollider::Load(Config& config)
{

}

void ConvexCollider::DebugShape() {
    
    if (!cookedMesh) return;

    physx::PxRigidActor* actor = (attachedRigidbody) ? attachedRigidbody->GetActor() : nullptr;
    physx::PxTransform worldPose;

    if (actor && shape) {
        worldPose = actor->getGlobalPose() * shape->getLocalPose();
    }
    else {
        glm::vec3 p = owner->transform->GetGlobalPosition();
        glm::quat r = owner->transform->GetGlobalQuaterionRotation();
        worldPose = physx::PxTransform(physx::PxVec3(p.x, p.y, p.z), physx::PxQuat(r.x, r.y, r.z, r.w));
    }

    glm::vec3 scale = owner->transform->GetGlobalScale();
    auto* render = Engine::GetInstance().moduleRender;
    glm::vec4 color = (actor && shape) ? glm::vec4(1, 1, 0, 1) : glm::vec4(1, 0, 0, 1);

    const physx::PxVec3* verts = cookedMesh->getVertices();
    const physx::PxU8* indexBuffer = cookedMesh->getIndexBuffer();
    uint32_t offset = 0;

    for (uint32_t i = 0; i < cookedMesh->getNbPolygons(); ++i) {
        physx::PxHullPolygon poly;
        cookedMesh->getPolygonData(i, poly);

        const physx::PxU8* indices = indexBuffer + poly.mIndexBase;
        for (uint32_t j = 0; j < poly.mNbVerts; ++j) {

            physx::PxVec3 p0 = verts[indices[j]];
            physx::PxVec3 p1 = verts[indices[(j + 1) % poly.mNbVerts]];

            p0 = worldPose.transform(p0.multiply(physx::PxVec3(scale.x, scale.y, scale.z)));
            p1 = worldPose.transform(p1.multiply(physx::PxVec3(scale.x, scale.y, scale.z)));

            render->DrawLine(glm::vec3(p0.x, p0.y, p0.z), glm::vec3(p1.x, p1.y, p1.z), color);
        }
    }
}

void ConvexCollider::OnGameObjectEvent(GameObjectEvent event, Component* component)
{
    switch (event)
    {
    case GameObjectEvent::MESH_CHANGED:

        CookMesh();
        break;
    }

    Collider::OnGameObjectEvent(event, component);
}