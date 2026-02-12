#include "MeshCollider.h"
#include "MeshRenderer.h"
#include "../resources/ResourceMesh.h"
#include "../GameObject.h"
#include "../Engine.h"
#include "../ModuleRender.h"
#include "Transform.h"
#include "Rigidbody.h"
#include "imgui.h"
#include "../utils/PhysicsCooker.h"

MeshCollider::MeshCollider(GameObject* owner) : Collider(owner) {
    
    name = "Mesh Collider";
    cookedMesh = nullptr;
    CookMesh();
}

MeshCollider::~MeshCollider() {
    
    if (cookedMesh) {
        cookedMesh->release();
        cookedMesh = nullptr;
    }
}

void MeshCollider::Update() {
    DebugShape();
}

physx::PxGeometry* MeshCollider::GetGeometry() {
    if (!cookedMesh) CookMesh();
    if (!cookedMesh) return nullptr;

    glm::vec3 scale = owner->transform->GetGlobalScale();
    return new physx::PxTriangleMeshGeometry(cookedMesh, physx::PxMeshScale(physx::PxVec3(scale.x, scale.y, scale.z)));
}

void MeshCollider::CookMesh() {
    MeshRenderer* meshRenderer = (MeshRenderer*)owner->GetComponent(ComponentType::MeshRenderer);

    bool hasValidMesh = meshRenderer && meshRenderer->GetMeshResource() && !meshRenderer->GetMeshResource()->vertices.empty();

    if (cookedMesh) {
        cookedMesh->release();
        cookedMesh = nullptr;
    }

    if (!hasValidMesh) {
        if (!meshRenderer) LOG(LogType::LOG_WARNING, "Mesh Collider on '%s' ignored: No MeshRenderer component found.", owner->name.c_str());
    }
    else {
        auto resource = meshRenderer->GetMeshResource();

        int vertexSize = resource->vertices.size();
        std::vector<glm::vec3> vertices(vertexSize);
        for (int i = 0; i < vertexSize; i++) {
            vertices[i] = resource->vertices[i].position;
        }

        int indicesSize = resource->indices.size();
        std::vector<uint32_t> indices(indicesSize);
        for (int i = 0; i < indicesSize; i++) {
            indices[i] = resource->indices[i];
        }

        cookedMesh = PhysicsCooker::CookTriangleMesh(
            (const float*)vertices.data(),
            (uint32_t)vertexSize,
            sizeof(glm::vec3),
            indices.data(),
            (uint32_t)indicesSize
        );
    }

    if (attachedRigidbody) attachedRigidbody->UpdateShapesGeometry();
}

void MeshCollider::OnEditor() {
    OnEditorBase();
    ImGui::Separator();

    if (cookedMesh) {
        ImGui::Text("Triangle Mesh Stats:");
        ImGui::BulletText("Vertices: %d", cookedMesh->getNbVertices());
        ImGui::BulletText("Triangles: %d", cookedMesh->getNbTriangles());
    }
    else {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "No mesh cooked!");
    }

    if (ImGui::Button("Recook Triangle Mesh")) {
        CookMesh();
        if (attachedRigidbody) attachedRigidbody->CreateBody();
    }
}

void MeshCollider::Save(Config& config) {
    SaveBase(config);
}

void MeshCollider::Load(Config& config) {
    LoadBase(config);
    CookMesh();
}

void MeshCollider::DebugShape() {
    
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
    const void* indices = cookedMesh->getTriangles();
    bool is16bit = cookedMesh->getTriangleMeshFlags() & physx::PxTriangleMeshFlag::e16_BIT_INDICES;

    for (uint32_t i = 0; i < cookedMesh->getNbTriangles(); ++i) {
        uint32_t v0, v1, v2;
        if (is16bit) {
            const uint16_t* i16 = (const uint16_t*)indices;
            v0 = i16[i * 3]; v1 = i16[i * 3 + 1]; v2 = i16[i * 3 + 2];
        }
        else {
            const uint32_t* i32 = (const uint32_t*)indices;
            v0 = i32[i * 3]; v1 = i32[i * 3 + 1]; v2 = i32[i * 3 + 2];
        }

        auto transformPt = [&](physx::PxVec3 p) {
            p = p.multiply(physx::PxVec3(scale.x, scale.y, scale.z));
            p = worldPose.transform(p);
            return glm::vec3(p.x, p.y, p.z);
            };

        render->DrawLine(transformPt(verts[v0]), transformPt(verts[v1]), color);
        render->DrawLine(transformPt(verts[v1]), transformPt(verts[v2]), color);
        render->DrawLine(transformPt(verts[v2]), transformPt(verts[v0]), color);
    }
}

void MeshCollider::OnGameObjectEvent(GameObjectEvent event, Component* component) {
    switch (event) {
    case GameObjectEvent::MESH_CHANGED:
        CookMesh();
        break;
    }
    Collider::OnGameObjectEvent(event, component);
}