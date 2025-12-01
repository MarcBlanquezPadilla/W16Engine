#pragma once
#include <string>
#include "Importer.h"

struct aiMesh;
class ResourceMesh;

class MeshImporter : public Importer
{
public:

    // Guarda un aiMesh de Assimp en formato binario personalizado
    static bool Save(const aiMesh* mesh, const char* destinationFile);

    // Escribe directamente un aiMesh a disco con un UID específico
    static bool WriteMeshToDisk(const aiMesh* mesh, uint32_t uid);
};