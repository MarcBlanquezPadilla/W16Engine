#include "Transform.h"
#include "../Log.h"
#include "Component.h"
#include <vector>
#include <assimp/scene.h>

Transform::Transform(GameObject* owner, bool enabled) : Component(owner, enabled)
{
    position = glm::vec3(0.0f, 0.0f, 0.0f);
    scale = glm::vec3(1.0f, 1.0f, 1.0f);
    rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
}

Transform::~Transform()
{
    
}

glm::mat4 Transform::GetLocalMatrix() const
{
    glm::mat4 matTranslation = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 matRotation = glm::mat4_cast(rotation);
    glm::mat4 matScale = glm::scale(glm::mat4(1.0f), scale);

    return matTranslation * matRotation * matScale;
}