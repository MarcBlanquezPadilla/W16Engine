#include "Mesh.h"
#include "../Log.h"
#include "Component.h"
#include "../GameObject.h"
#include <vector>
#include <assimp/scene.h>
#include "../Engine.h"
#include "../Render.h"
#include <fstream>
#include <cmath>

Mesh::Mesh(GameObject* owner, bool enabled) : Component(owner, enabled)
{

}

Mesh::~Mesh()
{
	
}

void Mesh::CleanUp()
{
	Engine::GetInstance().render->DeleteMeshFromGPU(this->meshData);
}
    
bool Mesh::LoadModel(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
{
    if (vertices.empty() || indices.empty()) {
        LOG("Error: Assimp mesh read but empty vectors.");
        return false;
    }

    meshData.numVertices = vertices.size();
    meshData.numIndices = indices.size();

    aabb.min = {INFINITY, INFINITY, INFINITY};
    aabb.max = {-INFINITY, -INFINITY, -INFINITY};

    for (const Vertex& vertex : vertices)
    {
        aabb.min.x = fmin(aabb.min.x, vertex.position.x);
        aabb.min.y = fmin(aabb.min.y, vertex.position.y);
        aabb.min.z = fmin(aabb.min.z, vertex.position.z);
        aabb.max.x = fmax(aabb.max.x, vertex.position.x);
        aabb.max.y = fmax(aabb.max.y, vertex.position.y);
        aabb.max.z = fmax(aabb.max.z, vertex.position.z);
    }

    if (!LoadToGpu(vertices, indices))
    {
        LOG("Error: Failed to upload mesh to GPU.");
        return false;
    }

    if (!LoadNormalsToGpu(vertices, indices))
    {
        LOG("Error: Failed to upload normals to GPU.");
    }

    if (!SaveToLibrary(vertices, indices))
    {
        LOG("Error: Failed saving to library.");
    }

    return true;
}

bool Mesh::LoadToGpu(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
{
    if (vertices.empty() || indices.empty())
    {
        LOG("Error: Vertices or indices were empty");
        return false;
    }

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

bool Mesh::SaveToLibrary(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
{
    libraryPath = "Library/Meshes/" + owner->name + ".W16Mesh";
    std::ofstream file(libraryPath, std::ios::out | std::ios::binary);

    if (!file.is_open())
    {
        LOG("Error: Could not open the .mesh file for writing: %s", libraryPath.c_str());
        return false;
    }

    uint32_t num_vertices = vertices.size();
    uint32_t num_indices = indices.size();

    file.write(reinterpret_cast<const char*>(&num_vertices), sizeof(uint32_t));

    file.write(reinterpret_cast<const char*>(&num_indices), sizeof(uint32_t));

    file.write(reinterpret_cast<const char*>(vertices.data()), num_vertices * sizeof(Vertex));

    file.write(reinterpret_cast<const char*>(indices.data()), num_indices * sizeof(unsigned int));

    file.close();

    LOG("Mesh saved in Library: %s", libraryPath.c_str());
    return true;
}

bool Mesh::LoadFromLibrary(std::string path)
{
    std::ifstream file(path, std::ios::in | std::ios::binary);

    if (!file.is_open())
    {
        LOG("Error: Could not open the .mesh file for reading: %s", path.c_str());
        return false;
    }

    uint32_t num_vertices = 0;
    uint32_t num_indices = 0;

    file.read(reinterpret_cast<char*>(&num_vertices), sizeof(uint32_t));
    file.read(reinterpret_cast<char*>(&num_indices), sizeof(uint32_t));

    if (num_vertices == 0 || num_indices == 0)
    {
        LOG("Error: Mesh file has 0 vertices or indices: %s", path.c_str());
        file.close();
        return false;
    }

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    vertices.resize(num_vertices);
    indices.resize(num_indices);

    file.read(reinterpret_cast<char*>(vertices.data()), num_vertices * sizeof(Vertex));
    file.read(reinterpret_cast<char*>(indices.data()), num_indices * sizeof(unsigned int));
    file.read(reinterpret_cast<char*>(vertices.data()), num_vertices * sizeof(Vertex));
    file.read(reinterpret_cast<char*>(indices.data()), num_indices * sizeof(unsigned int));
    file.close();

    LOG("Mesh loaded from Library: %s", path.c_str());

    LoadModel(vertices, indices);

    return true;
}


void Mesh::Save(pugi::xml_node componentNode)
{
    componentNode.append_attribute("type") = (int)GetType();
    componentNode.append_attribute("path") = libraryPath.c_str();
}

void Mesh::Load(pugi::xml_node componentNode)
{
    LoadFromLibrary(componentNode.attribute("path").as_string());
}