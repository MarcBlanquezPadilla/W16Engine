#pragma once
#include "glm/glm.hpp"
#include "../Global.h"
#include "../utils/Log.h"

struct Bone {
    std::string name;
    glm::mat4 offsetMatrix;
};