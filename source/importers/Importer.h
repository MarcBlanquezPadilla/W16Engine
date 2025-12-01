#pragma once
#include <string>
#include "../resources/Resource.h"

class Importer {
public:
    virtual bool Import(const std::string& assetPath, const std::string& libraryPath, uint32_t modelUID) = 0;

protected:
    // Helper simple para Texture, Audio, Script...
    // No es virtual, es solo una utilidad para no repetir código.
    void SaveBasicMeta(const std::string& path, uint32_t uid, ResourceType type) {
        // ... (código de escribir UID + Type) ...
    }
};