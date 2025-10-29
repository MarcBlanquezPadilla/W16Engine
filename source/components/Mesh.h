#pragma once
#include "Component.h"
#include <glm/glm.hpp>

class GameObject;

struct aiMesh;

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

public:
    MeshData meshData;
    bool hasUVs = false;
};