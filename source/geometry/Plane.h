#pragma once
#include "glm/glm.hpp"

struct Plane
{
    glm::vec3 normal;
    float distance;

    void Normalize();

    float GetDistanceToPoint(const glm::vec3& point) const;
};