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
    glm::vec2 texCoords;
};


struct MeshData
{
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;
    int numIndices = 0;
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

    bool LoadFromAssimpMesh(aiMesh* assimpMesh);

    bool LoadCube();
    bool LoadSphere();
    bool LoadPyramid();

    bool LoadToGpu(std::vector<Vertex> vertices, std::vector<unsigned int> indices);


public:
    MeshData meshData;
    bool hasUVs = false;
};