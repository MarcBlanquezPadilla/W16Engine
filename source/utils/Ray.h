#pragma once
#include <glm/glm.hpp>

class AABB;

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;

    bool RayIntersectsAABB(const AABB& aabb, float& t);
};