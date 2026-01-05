#pragma once
#include "Resource.h"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include <vector>
#include <string>

template <typename T>
struct AnimationKey
{
    double time;
    T value;
};

struct Channel
{
    std::string name;

    std::vector<AnimationKey<glm::vec3>> positionKeys;
    std::vector<AnimationKey<glm::quat>> rotationKeys;
    std::vector<AnimationKey<glm::vec3>> scaleKeys;
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