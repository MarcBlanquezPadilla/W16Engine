#pragma once
#include "Component.h"
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <cmath>

class AABB;
class GameObject;

struct aiMesh;
struct Vertex;

enum Basics
{
    Cube,
    Triangle,
    Sphere
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

struct StencilData
{
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    int numVertices = 0;
};


class Mesh : public Component
{
public:

    Mesh(GameObject* owner);

    ~Mesh() override;

    void CleanUp() override;
    
    ComponentType GetType() override {
        return ComponentType::Mesh;
    }

    void Save(Config& componentNode) override;
    void Load(Config& componentNode) override;

    bool SaveToLibrary(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    bool LoadFromLibrary(std::string path);

    bool LoadModel(std::vector<Vertex> vertices, std::vector<unsigned int> indices);

    std::vector<Vertex> GetVertices();
    std::vector<unsigned int> GetIndices();

    void OnEditor() override;

private:
    bool LoadToGpu(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
    bool LoadNormalsToGpu(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
    bool LoadSmothedNormalsToGpu(std::vector<Vertex> vertices, std::vector<unsigned int> indices);


public:
    MeshData meshData;
    NormalData normalData;
    StencilData stencilData;
    AABB* aabb;

    bool hasUVs = false;
    bool drawNormals = false;
    bool drawStencil = false;
    bool drawMesh = false;

    std::string libraryPath;

private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};