#include "Module.h"
#include "GameObject.h"
#include "components/Component.h"
#include "components/Mesh.h"
#include "components/Transform.h"
#include <list>
#include "Log.h"

GameObject::GameObject(bool _enabled) :enabled(_enabled)
{
	AddComponent(ComponentType::Transform);
}

GameObject::~GameObject()
{

}

bool GameObject::Awake()
{
	bool ret = true;

	return ret;
}

bool GameObject::Update(float dt)
{
	bool ret = true;

	for (auto const& pair : components)
	{
		Component* component = pair.second;
		if (component->enabled)
		{
			component->Update(dt);
		}
	}

	return ret;
}

bool GameObject::CleanUp()
{
	bool ret = true;

	return ret;
}

Component* GameObject::AddComponent(ComponentType type)
{
	// 1. ¡Comprobar si ya existe es mucho más fácil!
	if (components.count(type) > 0)
	{
		LOG("Error: Este GameObject ya tiene un componente de este tipo.");
		return components[type]; // Devolvemos el que ya existe
	}

	// 2. Creamos el nuevo componente (como antes)
	Component* component = nullptr;
	switch (type)
	{
	case ComponentType::None:
		break;
	case ComponentType::Transform:
		component = new Transform(this, true);
		break;
	case ComponentType::Mesh:
		component = new Mesh(this, true);
		break;
	case ComponentType::Texture:
		// component = new Texture(this, true);
		break;
	}

	// 3. Añadir al map (¡sintaxis mucho más limpia!)
	if (component != nullptr)
	{
		components[type] = component;
	}

	return component;
}

Component* GameObject::GetComponent(ComponentType type)
{

	if (components.count(type) > 0)
	{
		return components[type];
	}

	return nullptr;
}