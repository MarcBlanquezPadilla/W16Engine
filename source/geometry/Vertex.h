#pragma once
#include "glm/glm.hpp"
#include "../Global.h"
#include "../utils/Log.h"

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;

    int boneIDs[MAX_BONE_INFLUENCE] = { -1, -1, -1, -1 };

    float weights[MAX_BONE_INFLUENCE] = { 0.0f, 0.0f, 0.0f, 0.0f };

    void Vertex::AddBoneData(int boneID, float weight)
    {
        for (int i = 0; i < 4; i++)
        {
            if (boneIDs[i] == -1)
            {
                boneIDs[i] = boneID;
                weights[i] = weight;
                return;
            }
        }

        int smallestIndex = -1;
        float smallestWeight = weight;

        for (int i = 0; i < 4; i++)
        {
            if (weights[i] < smallestWeight)
            {
                smallestWeight = weights[i];
                smallestIndex = i;
            }
        }

        if (smallestIndex != -1)
        {
            boneIDs[smallestIndex] = boneID;
            weights[smallestIndex] = weight;
        }
    }
};