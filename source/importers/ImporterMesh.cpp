#include "ImporterMesh.h"
#include "../utils/Log.h"
#include <assimp/scene.h> 
#include <fstream>
#include <map>

bool ImporterMesh::Import(const std::string libraryPath, const UID uid, const int type, aiMesh* assimpMesh)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Bone> bonesInfo; // <--- LISTA PARA GUARDAR LOS DATOS DE HUESOS

    // 1. FILL VERTICES
    for (unsigned int i = 0; i < assimpMesh->mNumVertices; i++)
    {
        Vertex vertex;
        vertex.position = glm::vec3(assimpMesh->mVertices[i].x, assimpMesh->mVertices[i].y, assimpMesh->mVertices[i].z);

        if (assimpMesh->HasNormals())
            vertex.normal = glm::vec3(assimpMesh->mNormals[i].x, assimpMesh->mNormals[i].y, assimpMesh->mNormals[i].z);
        else
            vertex.normal = glm::vec3(0.0f);

        if (assimpMesh->HasTextureCoords(0))
            vertex.texCoords = glm::vec2(assimpMesh->mTextureCoords[0][i].x, assimpMesh->mTextureCoords[0][i].y);
        else
            vertex.texCoords = glm::vec2(0.0f);

        // Inicializamos los pesos a 0 y IDs a -1 por seguridad (aunque el constructor del struct ya lo hace)
        for (int k = 0; k < 4; k++) { vertex.boneIDs[k] = -1; vertex.weights[k] = 0.0f; }

        vertices.push_back(vertex);
    }

    // 2. FILL INDICES
    for (unsigned int i = 0; i < assimpMesh->mNumFaces; i++)
    {
        aiFace face = assimpMesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // 3. FILL BONES (SKINNING)
    std::map<std::string, int> boneMapping;
    int boneCount = 0;

    if (assimpMesh->HasBones())
    {
        for (unsigned int i = 0; i < assimpMesh->mNumBones; i++)
        {
            aiBone* bone = assimpMesh->mBones[i];
            std::string boneName = bone->mName.C_Str();

            int boneID = -1;

            // --- GESTIÓN DE MAPEO Y OFFSET MATRIX ---
            if (boneMapping.find(boneName) == boneMapping.end())
            {
                boneID = boneCount;
                boneCount++;
                boneMapping[boneName] = boneID;

                // Guardamos la info del hueso para el archivo binario
                Bone newBoneInfo;
                newBoneInfo.name = boneName;

                // CONVERSIÓN DE MATRIZ (Assimp row-major a GLM column-major)
                aiMatrix4x4 offset = bone->mOffsetMatrix;
                newBoneInfo.offsetMatrix = glm::mat4(
                    offset.a1, offset.b1, offset.c1, offset.d1,
                    offset.a2, offset.b2, offset.c2, offset.d2,
                    offset.a3, offset.b3, offset.c3, offset.d3,
                    offset.a4, offset.b4, offset.c4, offset.d4
                );

                bonesInfo.push_back(newBoneInfo);
            }
            else
            {
                boneID = boneMapping[boneName];
            }

            // --- GESTIÓN DE PESOS EN VÉRTICES ---
            for (unsigned int j = 0; j < bone->mNumWeights; j++)
            {
                int vertexId = bone->mWeights[j].mVertexId;
                float weight = bone->mWeights[j].mWeight;

                if (vertexId < vertices.size())
                {
                    vertices[vertexId].AddBoneData(boneID, weight);
                }
            }
        }
    }

    // Pasamos el nuevo vector bonesInfo a la función de guardado
    return Import(libraryPath, uid, type, vertices, indices, bonesInfo);
}

// --------------------------------------------------------------------------------
// FUNCIÓN DE GUARDADO BINARIO (ACTUALIZADA)
// --------------------------------------------------------------------------------
bool ImporterMesh::Import(const std::string libraryPath, const UID uid, const int type, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, std::vector<Bone>& bones)
{
    std::ofstream file(libraryPath, std::ios::out | std::ios::binary);
    if (file.is_open())
    {
        // 1. CABECERA
        file.write((const char*)&uid, sizeof(UID));
        file.write((const char*)&type, sizeof(int));

        // 2. TAMAÑOS
        uint32_t num_vertices = vertices.size();
        uint32_t num_indices = indices.size();
        uint32_t num_bones = bones.size(); // <--- NUEVO

        file.write((const char*)&num_vertices, sizeof(uint32_t));
        file.write((const char*)&num_indices, sizeof(uint32_t));
        file.write((const char*)&num_bones, sizeof(uint32_t)); // <--- NUEVO

        // 3. DATOS DE VÉRTICES (Ahora incluyen IDs y Weights dentro del struct Vertex)
        file.write((const char*)vertices.data(), num_vertices * sizeof(Vertex));

        // 4. DATOS DE ÍNDICES
        file.write((const char*)indices.data(), num_indices * sizeof(unsigned int));

        // 5. DATOS DE HUESOS (NUEVO)
        for (const auto& bone : bones)
        {
            // A. Nombre (Tamaño + Caracteres)
            uint32_t nameSize = bone.name.size();
            file.write((const char*)&nameSize, sizeof(uint32_t));
            file.write(bone.name.c_str(), nameSize);

            // B. Offset Matrix (64 bytes: 16 floats * 4 bytes)
            file.write((const char*)&bone.offsetMatrix, sizeof(glm::mat4));
        }

        file.close();
        LOG("Mesh imported with skinning data: %s (Bones: %d)", libraryPath.c_str(), num_bones);
        return true;
    }
    return false;
}