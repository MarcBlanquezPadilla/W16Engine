#include "ResourceMesh.h"
#include "../importers/MeshImporter.h"
#include "../utils/Log.h"
#include <fstream>

// Incluir OpenGL si lo usas
// #include <GL/glew.h>

ResourceMesh::~ResourceMesh()
{
    if (IsLoadedToMemory())
    {
        UnloadFromMemory_Internal();
    }
}

bool ResourceMesh::LoadInMemory()
{
    std::ifstream file(libraryFile, std::ios::binary);
    if (!file.is_open())
    {
        LOG("Error: Could not open library file %s", libraryFile);
        return false;
    }

    // === LEER CABECERA ===
    file.read(reinterpret_cast<char*>(&numVertices), sizeof(uint32_t));
    file.read(reinterpret_cast<char*>(&numIndices), sizeof(uint32_t));

    // === RESERVAR MEMORIA ===
    size_t positionsSize = sizeof(float) * numVertices * 3;
    size_t normalsSize = sizeof(float) * numVertices * 3;
    size_t uvsSize = sizeof(float) * numVertices * 2;
    size_t indicesSize = sizeof(uint32_t) * numIndices;

    float* positions = new float[numVertices * 3];
    float* normals = new float[numVertices * 3];
    float* uvs = new float[numVertices * 2];
    uint32_t* indices = new uint32_t[numIndices];

    // === LEER DATOS ===
    file.read(reinterpret_cast<char*>(positions), positionsSize);
    file.read(reinterpret_cast<char*>(normals), normalsSize);
    file.read(reinterpret_cast<char*>(uvs), uvsSize);
    file.read(reinterpret_cast<char*>(indices), indicesSize);

    file.close();

    // === CREAR BUFFER INTERLEAVED ===
    // Formato: Pos(3) + Normal(3) + UV(2) = 8 floats por vértice
    size_t vertexSize = sizeof(float) * numVertices * 8;
    float* vertices = new float[numVertices * 8];

    for (uint32_t i = 0; i < numVertices; i++)
    {
        // Posición
        vertices[i * 8 + 0] = positions[i * 3 + 0];
        vertices[i * 8 + 1] = positions[i * 3 + 1];
        vertices[i * 8 + 2] = positions[i * 3 + 2];

        // Normal
        vertices[i * 8 + 3] = normals[i * 3 + 0];
        vertices[i * 8 + 4] = normals[i * 3 + 1];
        vertices[i * 8 + 5] = normals[i * 3 + 2];

        // UV
        vertices[i * 8 + 6] = uvs[i * 2 + 0];
        vertices[i * 8 + 7] = uvs[i * 2 + 1];
    }

    // === SUBIR A GPU ===
    // Aquí deberías llamar a las funciones OpenGL
    // Ejemplo (descomenta y adapta según tu renderer):
    /*
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexSize, vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices, GL_STATIC_DRAW);

    // Posición (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    // Normal (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    // UV (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
    */

    // === LIMPIEZA ===
    delete[] positions;
    delete[] normals;
    delete[] uvs;
    delete[] indices;
    delete[] vertices;

    LOG("Mesh loaded successfully from %s", libraryFile);
    return true;
}

bool ResourceMesh::UnloadFromMemory_Internal()
{
    // Eliminar buffers de GPU
    if (vao != 0)
    {
        // Descomentar cuando tengas OpenGL configurado:
        // glDeleteVertexArrays(1, &vao);
        // glDeleteBuffers(1, &vbo);
        // glDeleteBuffers(1, &ebo);

        vao = vbo = ebo = 0;
        numVertices = 0;
        numIndices = 0;

        LOG("Mesh unloaded from GPU (UID: %u)", uid);
    }
    return true;
}