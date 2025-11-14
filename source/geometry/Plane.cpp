#pragma once
#include "Plane.h"

void Plane::Normalize()
{
    float length = glm::length(normal);
    normal /= length;
    distance /= length;
}

float Plane::GetDistanceToPoint(const glm::vec3& point) const
{
    return glm::dot(normal, point) + distance;
}