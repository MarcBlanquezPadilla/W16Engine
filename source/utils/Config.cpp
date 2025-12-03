#include "Config.h"
#include "Log.h" // Tu sistema de logs

Config::Config()
{
    rootDocument = new pugi::xml_document();

    node = rootDocument->append_child("Data");
}

Config::Config(pugi::xml_node node) : node(node), rootDocument(nullptr)
{

}

Config::~Config()
{
    if (rootDocument)
    {
        delete rootDocument;
        rootDocument = nullptr;
    }
}

bool Config::Load(const char* path)
{
    if (!rootDocument) rootDocument = new pugi::xml_document();

    pugi::xml_parse_result result = rootDocument->load_file(path);
    if (result)
    {
        node = rootDocument->first_child();
        return true;
    }
    else
    {
        LOG("Could not load config xml: %s. Error: %s", path, result.description());
        return false;
    }
}

bool Config::Save(const char* path)
{
    if (rootDocument)
    {
        return rootDocument->save_file(path);
    }

    return false;
}

Config Config::AddChild(const char* name)
{
    return Config(node.append_child(name));
}

Config Config::GetChild(const char* name) const
{
    return Config(node.child(name));
}

bool Config::IsValid() const
{
    return !node.empty();
}

void Config::SetInt(const char* name, int value)
{
    node.append_attribute(name).set_value(value);
}

void Config::SetUInt(const char* name, unsigned int value)
{
    node.append_attribute(name).set_value(value);
}

void Config::SetFloat(const char* name, float value)
{
    node.append_attribute(name).set_value(value);
}

void Config::SetBool(const char* name, bool value)
{
    node.append_attribute(name).set_value(value);
}

void Config::SetString(const char* name, const std::string& value)
{
    node.append_attribute(name).set_value(value.c_str());
}

void Config::SetVector3(const char* name, const glm::vec3& value)
{
    pugi::xml_node vecNode = node.append_child(name);
    vecNode.append_attribute("x").set_value(value.x);
    vecNode.append_attribute("y").set_value(value.y);
    vecNode.append_attribute("z").set_value(value.z);
}

int Config::GetInt(const char* name, int defaultValue) const
{
    return node.attribute(name).as_int(defaultValue);
}

unsigned int Config::GetUInt(const char* name, unsigned int defaultValue) const
{
    return node.attribute(name).as_uint(defaultValue);
}

float Config::GetFloat(const char* name, float defaultValue) const
{
    return node.attribute(name).as_float(defaultValue);
}

bool Config::GetBool(const char* name, bool defaultValue) const
{
    return node.attribute(name).as_bool(defaultValue);
}

std::string Config::GetString(const char* name, const std::string& defaultValue) const
{
    return node.attribute(name).as_string(defaultValue.c_str());
}

glm::vec3 Config::GetVector3(const char* name, const glm::vec3& defaultValue) const
{
    pugi::xml_node vecNode = node.child(name);
    if (!vecNode) return defaultValue;

    return glm::vec3(
        vecNode.attribute("x").as_float(defaultValue.x),
        vecNode.attribute("y").as_float(defaultValue.y),
        vecNode.attribute("z").as_float(defaultValue.z)
    );
}