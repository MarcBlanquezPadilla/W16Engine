#include "Mesh.h"
#include "../Log.h"
#include "Component.h"
#include <vector>
#include <assimp/scene.h>
#include "../Engine.h"
#include "../Render.h"

Mesh::Mesh(GameObject* owner, bool enabled) : Component(owner, enabled)
{

}

Mesh::~Mesh()
{
	
}

void Mesh::OnDestroy()
{
	Engine::GetInstance().render->DeleteMeshFromGPU(this->meshData);
}
    
bool Mesh::LoadFromAssimpMesh(aiMesh* assimpMesh)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	if (assimpMesh->HasTextureCoords(0))
	{
		this->hasUVs = true;
		LOG("Cargando malla CON coordenadas UV.");
	}
	else
	{
		this->hasUVs = false;
		LOG("Cargando malla SIN coordenadas UV. Se usarán UVs falsas.");
	}

	for (unsigned int i = 0; i < assimpMesh->mNumVertices; i++)
	{
		Vertex vertex;
		vertex.position.x = assimpMesh->mVertices[i].x;
		vertex.position.y = assimpMesh->mVertices[i].y;
		vertex.position.z = assimpMesh->mVertices[i].z;

		if (this->hasUVs)
		{
			vertex.texCoords.x = assimpMesh->mTextureCoords[0][i].x;
			vertex.texCoords.y = assimpMesh->mTextureCoords[0][i].y;
		}
		else
		{
			vertex.texCoords = glm::vec2(0.0f, 0.0f);
		}
		vertices.push_back(vertex);
	}


	for (unsigned int i = 0; i < assimpMesh->mNumFaces; i++) {
		aiFace face = assimpMesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	if (vertices.empty() || indices.empty()) {
		LOG("Error: La malla de Assimp no tiene vértices o índices.");
		return false;
	}

	return Engine::GetInstance().render->UploadMeshToGPU(this->meshData, vertices, indices);
}

bool Mesh::LoadCube()
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    vertices = {
        // Front face
        {glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 0.0f)},
        {glm::vec3(0.5f,  0.5f,  0.5f), glm::vec2(1.0f, 1.0f)},
        {glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(0.0f, 1.0f)},
        // Back face
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f)},
        {glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec2(1.0f, 1.0f)},
        {glm::vec3(0.5f,  0.5f, -0.5f), glm::vec2(0.0f, 1.0f)},
        // Left face
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 0.0f)},
        {glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(1.0f, 1.0f)},
        {glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec2(0.0f, 1.0f)},
        // Right face
        {glm::vec3(0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f)},
        {glm::vec3(0.5f,  0.5f, -0.5f), glm::vec2(1.0f, 1.0f)},
        {glm::vec3(0.5f,  0.5f,  0.5f), glm::vec2(0.0f, 1.0f)},
        // Top face
        {glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(0.5f,  0.5f,  0.5f), glm::vec2(1.0f, 0.0f)},
        {glm::vec3(0.5f,  0.5f, -0.5f), glm::vec2(1.0f, 1.0f)},
        {glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec2(0.0f, 1.0f)},
        // Bottom face
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f)},
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f)},
        {glm::vec3(0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 1.0f)},
        {glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 1.0f)}
    };

    indices = {
         0,  1,  2,  2,  3,  0, // Front
         4,  5,  6,  6,  7,  4, // Back
         8,  9, 10, 10, 11,  8, // Left
        12, 13, 14, 14, 15, 12, // Right
        16, 17, 18, 18, 19, 16, // Top
        20, 21, 22, 22, 23, 20  // Bottom
    };

    return LoadToGpu(vertices, indices);
}
bool Mesh::LoadSphere()
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    const int sectors = 36;
    const int stacks = 18;
    const float radius = 0.5f;

    for (int i = 0; i <= stacks; ++i)
    {
        float V = (float)i / (float)stacks;
        float phi = V * glm::pi<float>();

        for (int j = 0; j <= sectors; ++j)
        {
            float U = (float)j / (float)sectors;
            float theta = U * (glm::pi<float>() * 2);

            float x = cos(theta) * sin(phi);
            float y = cos(phi);
            float z = sin(theta) * sin(phi);

            vertices.push_back({ {x * radius, y * radius, z * radius}, {U, V} });
        }
    }

    for (int i = 0; i < stacks; ++i)
    {
        for (int j = 0; j < sectors; ++j)
        {
            int first = (i * (sectors + 1)) + j;
            int second = first + sectors + 1;

            indices.push_back(first);
            indices.push_back(first + 1);
            indices.push_back(second);

            indices.push_back(first + 1);
            indices.push_back(second + 1);
            indices.push_back(second);
        }
    }
    return LoadToGpu(vertices, indices);
}
bool Mesh::LoadPyramid()
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    vertices = {
        // Posición (x,y,z)          // UVs (x,y)
        // Base (Plano Y = -0.5)
        {glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 0.0f)}, // 0
        {glm::vec3(0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 0.0f)}, // 1
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 1.0f)}, // 2
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f)}, // 3

        // Cara Frontal (Z+)
        {glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 0.0f)}, // 4
        {glm::vec3(0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 0.0f)}, // 5
        {glm::vec3(0.0f,  0.5f,  0.0f), glm::vec2(0.5f, 1.0f)}, // 6 (Apex)

        // Cara Derecha (X+)
        {glm::vec3(0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 0.0f)}, // 7
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f)}, // 8
        {glm::vec3(0.0f,  0.5f,  0.0f), glm::vec2(0.5f, 1.0f)}, // 9 (Apex)

        // Cara Trasera (Z-)
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f)}, // 10
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f)}, // 11
        {glm::vec3(0.0f,  0.5f,  0.0f), glm::vec2(0.5f, 1.0f)}, // 12 (Apex)

        // Cara Izquierda (X-)
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f)}, // 13
        {glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 0.0f)}, // 14
        {glm::vec3(0.0f,  0.5f,  0.0f), glm::vec2(0.5f, 1.0f)}  // 15 (Apex)
    };

    // 6 triángulos en total (2 para la base, 1 para cada cara lateral)
    // 6 * 3 = 18 índices
    // (Definidos en sentido anti-horario, CCW)
    indices = {
         0,  1,  2,   0,  2,  3, // Base
         4,  5,  6,             // Front
         7,  8,  9,             // Right
        10, 11, 12,             // Back
        13, 14, 15              // Left
    };

    return LoadToGpu(vertices, indices);
}

bool Mesh::LoadToGpu(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
{
    if (!vertices.empty() && !indices.empty())
    {
        bool success = Engine::GetInstance().render->UploadMeshToGPU(meshData, vertices, indices);
        if (success)
        {
            hasUVs = true;
            LOG("Created basic shape");
            return true;
        }
        else
        {
            LOG("Error: Could not upload basic mesh to GPU.");
            return false;
        }
    }
    else
    {
        LOG("Error: Vertices or indices were empty");
        return false;
    }
}
