#include "Module.h"
#include "GameObject.h"
#include "components/Component.h"
#include "components/Mesh.h"
#include <list>

GameObject::GameObject(bool _enabled) :enabled(_enabled)
{

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

	for each(Component * component in components)
	{
		component->Update(dt);
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
	Component* component = nullptr;

	switch (type)
	{
		case ComponentType::None:
			break;
		case ComponentType::Transform:
			break;
		case ComponentType::Mesh:
			component = new Mesh(this, true);
			break;
		case ComponentType::Texture:
			break;
	}

	components.push_back(component);
	return component;
}