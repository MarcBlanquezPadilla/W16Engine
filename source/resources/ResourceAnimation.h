#pragma once
#include "Resource.h"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include <vector>
#include <string>

struct Channel
{
    std::string name;

    std::vector<glm::vec3> positionKeys;
    std::vector<glm::quat> rotationKeys;
    std::vector<glm::vec3> scaleKeys;
};

class ResourceAnimation : public Resource
{
public:
    ResourceAnimation(UID id);
    virtual ~ResourceAnimation();

    bool LoadToMemory_Internal() override;
    bool UnloadFromMemory_Internal() override;

    double duration = 0.0;
    double ticksPerSecond = 0.0;

    std::vector<Channel> channels;

    void CleanUp();
};