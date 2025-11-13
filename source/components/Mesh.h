#pragma once
#include "Component.h"
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <cmath>

class GameObject;

struct aiMesh;

enum Basics
{
    Cube,
    Triangle,
    Sphere
};

struct AABB
{
    glm::vec3 min;
    glm::vec3 max;

    AABB GetGlobalAABB(const glm::mat4& modelMatrix) const
    {
        // Usamos un array de C++ moderno (std::array) o un array de C (glm::vec3[8])
        // para evitar alocar memoria (es más rápido que std::vector)
        const std::array<glm::vec3, 8> localVertices = {
            glm::vec3(min.x, min.y, min.z),
            glm::vec3(max.x, min.y, min.z),
            glm::vec3(min.x, max.y, min.z),
            glm::vec3(max.x, max.y, min.z),
            glm::vec3(min.x, min.y, max.z),
            glm::vec3(max.x, min.y, max.z),
            glm::vec3(min.x, max.y, max.z),
            glm::vec3(max.x, max.y, max.z)
        };

        AABB globalAABB;

        globalAABB.min = glm::vec3(INFINITY, INFINITY, INFINITY);
        globalAABB.max = glm::vec3(-INFINITY, -INFINITY, -INFINITY);

        for (int i = 0; i < localVertices.size(); ++i)
        {
            glm::vec3 globalVertices = glm::vec3(modelMatrix * glm::vec4(localVertices[i], 1.0f));

            globalAABB.min.x = fmin(globalAABB.min.x, globalVertices.x);
            globalAABB.min.y = fmin(globalAABB.min.y, globalVertices.y);
            globalAABB.min.z = fmin(globalAABB.min.z, globalVertices.z);
            globalAABB.max.x = fmax(globalAABB.max.x, globalVertices.x);
            globalAABB.max.y = fmax(globalAABB.max.y, globalVertices.y);
            globalAABB.max.z = fmax(globalAABB.max.z, globalVertices.z);
        }

        return globalAABB;
    }
};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};


struct MeshData
{
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;
    int numIndices = 0;
    int numVertices = 0;
};

struct NormalData
{
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    int numVertices = 0; 
};


class Mesh : public Component
{
public:

    Mesh(GameObject* owner, bool enabled);

    ~Mesh() override;

    void CleanUp() override;
    
    ComponentType GetType() override {
        return ComponentType::Mesh;
    }

    void Save(pugi::xml_node componentNode) override;
    void Load(pugi::xml_node componentNode) override;

    bool SaveToLibrary(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    bool LoadFromLibrary(std::string path);

    bool LoadModel(std::vector<Vertex> vertices, std::vector<unsigned int> indices);



private:
    bool LoadToGpu(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
    bool LoadNormalsToGpu(std::vector<Vertex> vertices, std::vector<unsigned int> indices);


public:
    MeshData meshData;
    NormalData normalData;
    AABB aabb;

    bool hasUVs = false;
    bool drawNormals = false;

    std::string libraryPath;
};