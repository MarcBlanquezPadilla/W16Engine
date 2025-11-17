#include "Ray.h"
#include "AABB.h"

bool Ray::RayIntersectsAABB(const AABB& aabb, float& t) {
    float tmin = -FLT_MAX;
    float tmax = FLT_MAX;

    if (std::abs(this->direction.x) > 1e-8f) {
        float t1 = (aabb.min.x - this->origin.x) / this->direction.x;
        float t2 = (aabb.max.x - this->origin.x) / this->direction.x;
        tmin = std::max(tmin, std::min(t1, t2));
        tmax = std::min(tmax, std::max(t1, t2));
    }
    else {
        if (this->origin.x < aabb.min.x || this->origin.x > aabb.max.x)
            return false;
    }

    if (std::abs(this->direction.y) > 1e-8f) {
        float t1 = (aabb.min.y - this->origin.y) / this->direction.y;
        float t2 = (aabb.max.y - this->origin.y) / this->direction.y;
        tmin = std::max(tmin, std::min(t1, t2));
        tmax = std::min(tmax, std::max(t1, t2));
    }
    else {
        if (this->origin.y < aabb.min.y || this->origin.y > aabb.max.y)
            return false;
    }

    if (std::abs(this->direction.z) > 1e-8f) {
        float t1 = (aabb.min.z - this->origin.z) / this->direction.z;
        float t2 = (aabb.max.z - this->origin.z) / this->direction.z;
        tmin = std::max(tmin, std::min(t1, t2));
        tmax = std::min(tmax, std::max(t1, t2));
    }
    else {
        if (this->origin.z < aabb.min.z || this->origin.z > aabb.max.z)
            return false;
    }

    if (tmax < tmin || tmax < 0)
        return false;

    t = (tmin >= 0) ? tmin : tmax;
    return true;
}