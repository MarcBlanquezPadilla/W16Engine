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
        // 1. PRIMER INTENTO: BUSCAR HUECO LIBRE
        for (int i = 0; i < 4; i++) // Asumiendo MAX_BONE_INFLUENCE = 4
        {
            if (boneIDs[i] == -1)
            {
                boneIDs[i] = boneID;
                weights[i] = weight;
                return; // Entró limpio, nos vamos.
            }
        }

        // 2. SEGUNDO INTENTO: EL ARRAY ESTÁ LLENO, ¿VALE LA PENA ENTRAR?
        // Buscamos cuál es el hueso más débil que tenemos guardado
        int smallestIndex = -1;
        float smallestWeight = weight; // Empezamos comparando con el nuevo

        for (int i = 0; i < 4; i++)
        {
            if (weights[i] < smallestWeight)
            {
                smallestWeight = weights[i];
                smallestIndex = i;
            }
        }

        // Si encontramos uno más débil que el nuevo, lo reemplazamos
        if (smallestIndex != -1)
        {
            boneIDs[smallestIndex] = boneID;
            weights[smallestIndex] = weight;
        }

        // Si no encontramos ninguno más débil (smallestIndex sigue siendo -1),
        // significa que el nuevo peso es basura comparado con los que ya tenemos.
        // Lo descartamos y no hacemos nada.
    }
};