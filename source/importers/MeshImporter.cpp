#include "MeshImporter.h"
#include "../resources/ResourceMesh.h"
#include "../utils/Log.h"
#include "../utils/FileUtils.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <fstream>

bool MeshImporter::Save(const aiMesh* mesh, const char* destinationFile)
{
    if (!mesh) return false;

    uint32_t numVertices = mesh->mNumVertices;
    uint32_t numIndices = 0;

    if (mesh->HasFaces())
    {
        numIndices = mesh->mNumFaces * 3;
    }

    std::ofstream file(destinationFile, std::ios::binary);
    if (!file.is_open())
    {
        LOG("Error: Could not create file %s", destinationFile);
        return false;
    }

    // === CABECERA ===
    file.write(reinterpret_cast<const char*>(&numVertices), sizeof(uint32_t));
    file.write(reinterpret_cast<const char*>(&numIndices), sizeof(uint32_t));

    // === POSICIONES ===
    file.write(reinterpret_cast<const char*>(mesh->mVertices), sizeof(float) * numVertices * 3);

    // === NORMALES ===
    if (mesh->HasNormals())
    {
        file.write(reinterpret_cast<const char*>(mesh->mNormals), sizeof(float) * numVertices * 3);
    }
    else
    {
        // Escribir normales vacías (ceros)
        float* emptyNormals = new float[numVertices * 3]();
        file.write(reinterpret_cast<const char*>(emptyNormals), sizeof(float) * numVertices * 3);
        delete[] emptyNormals;
    }

    // === UVs ===
    if (mesh->HasTextureCoords(0))
    {
        for (unsigned int i = 0; i < numVertices; i++)
        {
            float uv[2] = { 
                mesh->mTextureCoords[0][i].x, 
                mesh->mTextureCoords[0][i].y 
            };
            file.write(reinterpret_cast<const char*>(uv), sizeof(float) * 2);
        }
    }
    else
    {
        // Escribir UVs vacías
        float* emptyUVs = new float[numVertices * 2]();
        file.write(reinterpret_cast<const char*>(emptyUVs), sizeof(float) * numVertices * 2);
        delete[] emptyUVs;
    }

    // === ÍNDICES ===
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        const aiFace& face = mesh->mFaces[i];
        
        // Asegurar que es un triángulo
        if (face.mNumIndices == 3)
        {
            file.write(reinterpret_cast<const char*>(face.mIndices), sizeof(unsigned int) * 3);
        }
    }

    file.close();
    LOG("Mesh saved successfully: %s", destinationFile);
    return true;
}

bool MeshImporter::WriteMeshToDisk(const aiMesh* mesh, uint32_t uid)
{
    if (!mesh) return false;

    // Generar la ruta usando el UID
    std::string uidStr = std::to_string(uid);
    std::string folder = (uidStr.length() >= 2) ? uidStr.substr(0, 2) : "00";
    std::string directoryPath = "Library/" + folder;
    
    CreateDirectory(directoryPath);
    
    std::string libraryPath = directoryPath + "/" + uidStr + ".bin";

    return Save(mesh, libraryPath.c_str());
}