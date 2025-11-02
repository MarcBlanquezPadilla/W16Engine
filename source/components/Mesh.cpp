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
    
bool Mesh::LoadFromAssimpMesh(aiMesh* assimpMesh)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    //FILL VERTEXS
    for (unsigned int i = 0; i < assimpMesh->mNumVertices; i++)
    {
        Vertex vertex;

        vertex.position.x = assimpMesh->mVertices[i].x;
        vertex.position.y = assimpMesh->mVertices[i].y;
        vertex.position.z = assimpMesh->mVertices[i].z;

        if (assimpMesh->HasNormals()) {
            vertex.normal.x = assimpMesh->mNormals[i].x;
            vertex.normal.y = assimpMesh->mNormals[i].y;
            vertex.normal.z = assimpMesh->mNormals[i].z;
        }
        else {
            vertex.normal = glm::vec3(0.0f);
        }

        if (assimpMesh->HasTextureCoords(0)) {
            vertex.texCoords.x = assimpMesh->mTextureCoords[0][i].x;
            vertex.texCoords.y = assimpMesh->mTextureCoords[0][i].y;
            this->hasUVs = true;
        }
        else {
            vertex.texCoords = glm::vec2(0.0f);
            this->hasUVs = false;
        }

        vertices.push_back(vertex);
    }

    //FILL INDEX
    for (unsigned int i = 0; i < assimpMesh->mNumFaces; i++)
    {
        aiFace face = assimpMesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    if (vertices.empty() || indices.empty()) {
        LOG("Error: Malla de Assimp leída pero vectores vacíos.");
        return false;
    }

    this->normalData.numVertices = vertices.size();
    this->meshData.numIndices = indices.size();

    //UPLOAD TO GPU
    bool success = Engine::GetInstance().render->UploadMeshToGPU(this->meshData, vertices, indices);
    if (!success) {
        LOG("Error: Fallo al subir la malla principal de Assimp a la GPU.");
        return false;
    }

    //NORMALS
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

bool Mesh::LoadCube()
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    //CUBE CONSTRUCTION
    vertices = {

        {glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)},
        {glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
        {glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)},

        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f)},
        {glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)},
        {glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f)},

        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
        {glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
        {glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)},

        {glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
        {glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
        {glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)},

        {glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
        {glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
        {glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
        {glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
        {glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)}
    };

    indices = {
        0, 1, 2,  2, 3, 0,
        4, 5, 6,  6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };

    this->hasUVs = true;
    this->normalData.numVertices = vertices.size();
    this->meshData.numIndices = indices.size();

    
    return LoadToGpu(vertices, indices);
}
bool Mesh::LoadSphere()
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    //SPHERE CONSTRUCTION
    const int sectors = 36;
    const int stacks = 18;
    const float radius = 0.5f;

    for (int i = 0; i <= stacks; ++i) {
        float V = (float)i / (float)stacks;
        float phi = V * glm::pi<float>();
        for (int j = 0; j <= sectors; ++j) {
            float U = (float)j / (float)sectors;
            float theta = U * (glm::pi<float>() * 2);
            float x = cos(theta) * sin(phi);
            float y = cos(phi);
            float z = sin(theta) * sin(phi);
            vertices.push_back({
                {x * radius, y * radius, z * radius},
                {x, y, z},
                {U, V}
                });
        }
    }
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < sectors; ++j) {
            int first = (i * (sectors + 1)) + j;
            int second = first + sectors + 1;
            indices.push_back(first);
            indices.push_back(first + 1);
            indices.push_back(second);
            indices.push_back(first + 1);
            indices.push_back(second + 1);
            indices.push_back(second);
        }
    }

    this->hasUVs = true;
    this->normalData.numVertices = vertices.size();
    this->meshData.numIndices = indices.size();

    //LOAD TO GPU
    return LoadToGpu(vertices, indices);
}
bool Mesh::LoadPyramid()
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    glm::vec3 apex = glm::vec3(0.0f, 0.5f, 0.0f);

    //PYRAMID CONSTRUCTION
    vertices = {
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
        {glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
        {glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},

        {glm::vec3(-0.5f, -0.5f,  0.5f), glm::normalize(glm::vec3(0.0f, 0.5f, 0.5f)), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(0.5f, -0.5f,  0.5f), glm::normalize(glm::vec3(0.0f, 0.5f, 0.5f)), glm::vec2(1.0f, 0.0f)},
        {apex, glm::normalize(glm::vec3(0.0f, 0.5f, 0.5f)), glm::vec2(0.5f, 1.0f)},

        {glm::vec3(0.5f, -0.5f,  0.5f), glm::normalize(glm::vec3(0.5f, 0.5f, 0.0f)), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::normalize(glm::vec3(0.5f, 0.5f, 0.0f)), glm::vec2(1.0f, 0.0f)},
        {apex, glm::normalize(glm::vec3(0.5f, 0.5f, 0.0f)), glm::vec2(0.5f, 1.0f)},

        {glm::vec3(0.5f, -0.5f, -0.5f), glm::normalize(glm::vec3(0.0f, 0.5f, -0.5f)), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::normalize(glm::vec3(0.0f, 0.5f, -0.5f)), glm::vec2(1.0f, 0.0f)},
        {apex, glm::normalize(glm::vec3(0.0f, 0.5f, -0.5f)), glm::vec2(0.5f, 1.0f)},

        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::normalize(glm::vec3(-0.5f, 0.5f, 0.0f)), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(-0.5f, -0.5f,  0.5f), glm::normalize(glm::vec3(-0.5f, 0.5f, 0.0f)), glm::vec2(1.0f, 0.0f)},
        {apex, glm::normalize(glm::vec3(-0.5f, 0.5f, 0.0f)), glm::vec2(0.5f, 1.0f)},
    };

    indices = {
        0, 1, 3,  1, 2, 3,
        4, 5, 6,
        7, 8, 9,
        10, 11, 12,
        13, 14, 15
    };

    
    this->hasUVs = true;
    this->normalData.numVertices = vertices.size();
    this->meshData.numIndices = indices.size();

    //LOAD TO GPU
    return LoadToGpu(vertices, indices);
}

bool Mesh::LoadToGpu(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
{
    if (vertices.empty() || indices.empty())
    {
        LOG("Error: Vertices or indices were empty");
        return false;
    }

    //UPLOAD MESH TO GPU
    this->normalData.numVertices = vertices.size();
    this->meshData.numIndices = indices.size();

    bool success = Engine::GetInstance().render->UploadMeshToGPU(meshData, vertices, indices);

    if (!success)
    {
        LOG("Error: Could not upload basic mesh to GPU.");
        return false;
    }

    
    //UPLOAD NORMALS TO GPU
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


    hasUVs = true;
    LOG("Created basic shape and normal lines");
    return true;
}
