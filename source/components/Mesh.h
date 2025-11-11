#pragma once
#include "Component.h"
#include <glm/glm.hpp>
#include <vector>

class GameObject;

struct aiMesh;

enum Basics
{
    Cube,
    Triangle,
    Sphere
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

    void OnDestroy() override;
    
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
    bool hasUVs = false;
    bool drawNormals = false;
    std::string libraryPath;
};