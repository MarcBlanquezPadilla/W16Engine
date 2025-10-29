#pragma once
#include "Component.h"
#include <glm/glm.hpp>
#include "glm/gtc/quaternion.hpp"

class GameObject;

class Transform : public Component
{
public:

    Transform(GameObject* owner, bool enabled);

    virtual ~Transform() override;
    
    ComponentType GetType() override;

    glm::mat4 GetLocalMatrix() const;

public:
    glm::vec3 position;
    glm::vec3 scale;
    glm::quat rotation;
};