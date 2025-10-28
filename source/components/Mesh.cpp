#include "Mesh.h"
#include "../Log.h"
#include "Component.h"

Mesh::Mesh(GameObject* owner, bool enabled) : Component(owner, enabled)
{

}

Mesh::~Mesh()
{

}

void Mesh::Start()
{

}

void Mesh::Update(float dt)
{
    LOG("HOLA DESDE MESH");
}

void Mesh::OnDestroy()
{

}

void Mesh::OnEnable()
{

}
    
void Mesh::OnDisable()
{

}
    

ComponentType Mesh::GetType()
{
    return ComponentType::Mesh;
}