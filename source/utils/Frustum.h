#pragma once
#include "glm/glm.hpp"
#include <array>
#include "../geometry/Plane.h"
#include "AABB.h"

class Frustum
{
public:

    Frustum();
    ~Frustum();

    void Update(const glm::mat4& viewProjMatrix);

    bool InFrustum(const AABB& aabb) const;
    
private:
    std::array<Plane, 6> planes = {};
};