#pragma once
#include "Importer.h"

#include <string>
#include <vector>
#include <map>

struct aiNode;
struct aiScene;

class ModelImporter : public Importer
{
public:
    bool Import(const std::string& assetPath, const std::string& libraryPath, uint32_t modelUID) override;

private:

    // Ahora pasamos el mapa 'uidCache' por referencia
    void ProcessNode(aiNode * node, const aiScene * scene, std::vector<uint32_t>&meshUids, std::map<std::string, uint32_t>&uidCache);

    // Helpers para la persistencia del meta extendido
    void LoadMetaInfo(const std::string& metaPath, std::map<std::string, uint32_t>& uidCache);
    void SaveMetaInfo(const std::string& metaPath, uint32_t modelUID, const std::map<std::string, uint32_t>& uidCache);

    struct Vertex {
        float position[3];
        float normal[3];
        float texCoords[2];
    };
};