#include "GameObject.h"
#include "components/Component.h"
#include "components/Mesh.h"
#include "components/Transform.h"
#include "components/Texture.h"
#include <list>
#include "Log.h"

GameObject::GameObject(bool _enabled, std::string _name) : enabled(_enabled), name(_name)
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
	if (components.count(type) > 0)
	{
		LOG("Error: This GameObject already has a component of this type.");
		return components[type];
	}

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
		component = new Texture(this, true);
		break;
	}

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