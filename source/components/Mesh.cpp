#include "Mesh.h"
#include "../Log.h"
#include "Component.h"
#include <vector>
#include <assimp/scene.h>
#include "../Engine.h"
#include "../Render.h"

Mesh::Mesh(GameObject* owner, bool enabled) : Component(owner, enabled)
{

}

Mesh::~Mesh()
{
	
}

void Mesh::OnDestroy()
{
	Engine::GetInstance().render->DeleteMeshFromGPU(this->meshData);
}
    
bool Mesh::LoadModel(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
{
    if (vertices.empty() || indices.empty()) {
        LOG("Error: Assimp mesh read but empty vectors.");
        return false;
    }

    if (!LoadToGpu(vertices, indices))
        LOG("Error: Failed to upload mesh to GPU.");

    if (!LoadNormalsToGpu(vertices, indices))
        LOG("Error: Failed to upload normals to GPU.");
}

bool Mesh::LoadToGpu(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
{
    if (vertices.empty() || indices.empty())
    {
        LOG("Error: Vertices or indices were empty");
        return false;
    }

    //UPLOAD MESH TO GPU
    this->meshData.numVertices = vertices.size();
    this->meshData.numIndices = indices.size();

    bool success = Engine::GetInstance().render->UploadMeshToGPU(meshData, vertices, indices);

    if (!success)
    {
        LOG("Error: Could not upload basic mesh to GPU.");
        return false;
    }

    hasUVs = true;
    return true;
}

bool Mesh::LoadNormalsToGpu(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
{
    std::vector<glm::vec3> normal_lines;
    normal_lines.reserve(vertices.size() * 2);
    const float NORMAL_LINE_LENGTH = 0.5f;

    for (const Vertex& v : vertices)
    {
        normal_lines.push_back(v.position);
        glm::vec3 lineEnd = v.position + (glm::normalize(v.normal) * NORMAL_LINE_LENGTH);
        normal_lines.push_back(lineEnd);
    }

    if (!normal_lines.empty())
    {
        Engine::GetInstance().render->UploadLinesToGPU(
            this->normalData.VAO,
            this->normalData.VBO,
            normal_lines
        );
        this->normalData.numVertices = normal_lines.size();
    }
    return true;
}
