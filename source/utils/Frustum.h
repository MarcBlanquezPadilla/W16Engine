#pragma once
#include "glm/glm.hpp"
#include <array>
#include "../geometry/Plane.h"
#include "AABB.h"

class Frustum
{
public:

    std::array<Plane, 6> planes;

    void Update(const glm::mat4& viewProjMatrix);

    bool InFrustum(const AABB& aabb) const;
};