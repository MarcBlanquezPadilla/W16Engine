#pragma once

#include <string>
#include <vector>
#include "glm/glm.hpp"
#include "pugixml.hpp"

class Config
{
public:
    Config();

    Config(pugi::xml_node node);

    ~Config();

    bool Load(const char* path);
    bool Save(const char* path);

    Config AddChild(const char* name);
    Config GetChild(const char* name) const;
    bool IsValid() const;

    void SetInt(const char* name, int value);
    void SetInt64(const char* name, int64_t value);
    void SetUInt(const char* name, unsigned int value);
    void SetFloat(const char* name, float value);
    void SetBool(const char* name, bool value);
    void SetString(const char* name, const std::string& value);
    void SetVector3(const char* name, const glm::vec3& value);

    int GetInt(const char* name, int defaultValue = 0) const;
    int64_t GetInt64(const char* name, int64_t defaultValue = 0) const;
    unsigned int GetUInt(const char* name, unsigned int defaultValue = 0) const;
    float GetFloat(const char* name, float defaultValue = 0.0f) const;
    bool GetBool(const char* name, bool defaultValue = false) const;
    std::string GetString(const char* name, const std::string& defaultValue = "") const;
    glm::vec3 GetVector3(const char* name, const glm::vec3& defaultValue = glm::vec3(0.0f)) const;

private:
    pugi::xml_document* rootDocument = nullptr; // Solo existe si soy la raíz
    pugi::xml_node node; // El nodo actual que estoy manipulando
};