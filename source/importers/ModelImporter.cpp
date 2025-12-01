
#include "ModelImporter.h"
#include "../utils/Log.h"
#include "../utils/FileUtils.h"
#include "MeshImporter.h"
#include "../Resources.h"
#include "../Engine.h"

#include <fstream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

bool ModelImporter::Import(const std::string& assetPath, const std::string& libraryPath, uint32_t modelUID)
{
    std::map<std::string, uint32_t> uidCache;
    std::string metaPath = assetPath + ".meta";
    LoadMetaInfo(metaPath, uidCache);

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(assetPath,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        LOG("Error loading model with Assimp: %s", importer.GetErrorString());
        return false;
    }

    LOG("Importing model: %s", assetPath.c_str());
    LOG("  - Meshes: %d", scene->mNumMeshes);
    LOG("  - Materials: %d", scene->mNumMaterials);
    LOG("  - Textures: %d", scene->mNumTextures);

    // Vector donde guardaremos todos los UIDs de las mallas generadas
    std::vector<uint32_t> generatedMeshUids;

    // Procesar recursivamente todos los nodos
    ProcessNode(scene->mRootNode, scene, generatedMeshUids, uidCache);

    // Opcional: Aquí podrías guardar información del modelo completo
    // en el archivo libraryPath (jerarquía, transformaciones, etc.)
    // Por ahora solo devolvemos los UIDs de las mallas

    std::ofstream file(libraryPath, std::ios::binary);
    if (file.is_open()) {
        // Más adelante aquí guardarás la jerarquía de nodos.
        // Por ahora, guarda cuántos meshes tiene para no dejarlo vacío.
        uint32_t count = generatedMeshUids.size();
        file.write((char*)&count, sizeof(uint32_t));
        file.close();
    }

    LOG("Model imported successfully: %d meshes generated", (int)generatedMeshUids.size());

    return true;
}

void ModelImporter::ProcessNode(aiNode* node, const aiScene* scene, std::vector<uint32_t>& meshUids, std::map<std::string, uint32_t>& uidCache)
{
    // Procesar todas las mallas de este nodo
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* assimpMesh = scene->mMeshes[node->mMeshes[i]];
        std::string meshName = assimpMesh->mName.C_Str();

        // IMPORTANTE: Assimp a veces deja nombres vacíos. Si pasa, usa el índice como nombre.
        if (meshName.empty()) meshName = "Mesh_" + std::to_string(node->mMeshes[i]);

        uint32_t meshUID = 0;

        // 4. Verificamos persistencia
        if (uidCache.find(meshName) != uidCache.end()) {
            meshUID = uidCache[meshName]; // ¡Lo encontramos! Reutilizamos UID.
            // LOG("Reloading existing mesh: %s (UID: %u)", meshName.c_str(), meshUID);
        }
        else {
            meshUID = Engine::GetInstance().resources->GenerateNewUID(); // Nuevo mesh detectado
            uidCache[meshName] = meshUID; // Lo guardamos en caché
            LOG("New mesh detected: %s (UID: %u)", meshName.c_str(), meshUID);
        }

        // Guardamos a disco (Library) usando el UID (sea viejo o nuevo)
        if (MeshImporter::WriteMeshToDisk(assimpMesh, meshUID)) {
            meshUids.push_back(meshUID);
        }
    }

    // Procesar recursivamente los nodos hijos
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene, meshUids, uidCache);
    }
}

void ModelImporter::LoadMetaInfo(const std::string& metaPath, std::map<std::string, uint32_t>& uidCache)
{
    std::ifstream file(metaPath, std::ios::binary);
    if (!file.is_open()) return;

    // Saltamos la cabecera estándar que escribe Resources::SaveMeta 
    // (UID principal + Type) -> sizeof(uint32_t) + sizeof(int)
    file.seekg(sizeof(uint32_t) + sizeof(int));

    // Leemos cuántos sub-recursos hay
    uint32_t mapSize = 0;
    if (file.read(reinterpret_cast<char*>(&mapSize), sizeof(uint32_t)))
    {
        for (uint32_t i = 0; i < mapSize; ++i) {
            uint32_t storedUID = 0;
            uint32_t nameLength = 0;

            // Leer UID
            file.read(reinterpret_cast<char*>(&storedUID), sizeof(uint32_t));

            // Leer longitud del nombre
            file.read(reinterpret_cast<char*>(&nameLength), sizeof(uint32_t));

            // Leer string
            std::string name(nameLength, '\0');
            file.read(&name[0], nameLength);

            uidCache[name] = storedUID;
        }
    }
    file.close();
}

void ModelImporter::SaveMetaInfo(const std::string& metaPath, uint32_t modelUID, const std::map<std::string, uint32_t>& uidCache)
{
    // Sobrescribimos el meta completo
    std::ofstream file(metaPath, std::ios::binary);
    if (!file.is_open()) return;

    // 1. Cabecera Estándar (Compatible con Resources::LoadFile)
    ResourceType type = ResourceType::MODEL; // Asegúrate de tener acceso a este enum o castéalo
    file.write(reinterpret_cast<const char*>(&modelUID), sizeof(uint32_t));
    file.write(reinterpret_cast<const char*>(&type), sizeof(int));

    // 2. Datos extra del Modelo (Mapa de sub-recursos)
    uint32_t mapSize = (uint32_t)uidCache.size();
    file.write(reinterpret_cast<const char*>(&mapSize), sizeof(uint32_t));

    for (const auto& pair : uidCache) {
        // Escribir UID
        file.write(reinterpret_cast<const char*>(&pair.second), sizeof(uint32_t));

        // Escribir longitud nombre
        uint32_t nameLen = (uint32_t)pair.first.size();
        file.write(reinterpret_cast<const char*>(&nameLen), sizeof(uint32_t));

        // Escribir caracteres nombre
        file.write(pair.first.c_str(), nameLen);
    }

    file.close();
}