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

    // Pesos: ¿Cuánto le afecta? (ej: 0.8 al hueso 0, 0.2 al hueso 5)
    float weights[MAX_BONE_INFLUENCE] = { 0.0f, 0.0f, 0.0f, 0.0f };

    // Función auxiliar para añadir datos fácilmente desde el Importer
    void AddBoneData(int boneID, float weight)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            if (boneIDs[i] == -1) // Buscamos hueco libre
            {
                boneIDs[i] = boneID;
                weights[i] = weight;
                return;
            }
        }
        
        LOG("Vertex exceeds max bone influence limit (%d). Discarding lowest weights.", MAX_BONE_INFLUENCE);
    }
};