#include "GameObject.h"
#include "Scene.h"
#include "Engine.h"
#include "EventSystem.h"
#include "components/Component.h"
#include "components/Mesh.h"
#include "components/Transform.h"
#include "components/Texture.h"
#include "components/Camera.h"
#include "utils/Log.h"
#include "utils/AABB.h"

#include <random>

uint32_t GenerateUUID()
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<uint32_t> dis(1, UINT32_MAX);
	return dis(gen);
}

GameObject::GameObject(bool _enabled, std::string _name) : enabled(_enabled), name(_name)
{
	UUID = GenerateUUID();
	parentUUID = 0;
	parent = nullptr;
	transform = nullptr;
	childs.clear();
	components.clear();
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

	for (auto child : childs)
	{
		child->Update(dt);
	}

	return ret;
}

bool GameObject::CleanUp()
{
	bool ret = true;

	for each(auto pair in components)
	{
		pair.second->CleanUp();
		delete pair.second;
	}

	components.clear();

	for (int i = 0; i < childs.size(); i++)
	{
		childs[i]->CleanUp();
		delete childs[i];
	}
	childs.clear();

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
		component = new Transform(this);
		transform = (Transform*)component;
		break;
	case ComponentType::Mesh:
		component = new Mesh(this);
		break;
	case ComponentType::Texture:
		component = new Texture(this);
		break;
	case ComponentType::Camera:
		component = new Camera(this);
		break;
	}

	if (component != nullptr)
	{
		components[type] = component;
		component->Start();
		component->OnEnable();
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

bool GameObject::TryGetComponent(ComponentType type, Component*& outComponent)
{
	outComponent = nullptr;
	if (components.count(type) > 0)
	{
		outComponent = components[type];
		return true;
	}
	return false;
}

void GameObject::AddChild(GameObject* gameObject)
{
	std::string baseName = gameObject->name;
	std::string newName = baseName;
	int counter = 1;

	while (true)
	{
		bool nameCollision = false;


		for (GameObject* child : childs)
		{
			if (child->name == newName)
			{
				nameCollision = true;
				break;
			}
		}

		if (!nameCollision)
		{
			break;
		}

		newName = baseName + " (" + std::to_string(counter) + ")";
		counter++;
	}
	gameObject->name = newName;
	gameObject->parentUUID = UUID;
	gameObject->parent = this;
	childs.push_back(gameObject);
}

void GameObject::Save(pugi::xml_node gameObjectNode)
{
	gameObjectNode.append_attribute("Name") = name.c_str();
	gameObjectNode.append_attribute("UID") = UUID;
	gameObjectNode.append_attribute("ParentUID") = parentUUID;
	gameObjectNode.append_attribute("Enabled") = enabled;
	gameObjectNode.append_attribute("Static") = isStatic;

	if (components.size() > 0)
	{
		pugi::xml_node componentsNode = gameObjectNode.append_child("Components");
		for (auto const& pair : components)
		{
			pugi::xml_node compoenntNode = componentsNode.append_child("Component");
			Component* component = pair.second;
			if (component->enabled)
			{
				component->Save(compoenntNode);
			}
		}
	}

	if (childs.size() > 0)
	{
		pugi::xml_node childsNode = gameObjectNode.append_child("Childs");
		for (GameObject* child : childs)
		{
			pugi::xml_node childNode = childsNode.append_child("GameObject");
			child->Save(childNode);
		}
	}
}

void GameObject::Load(pugi::xml_node gameObjectNode)
{
	name = gameObjectNode.attribute("Name").as_string();
	UUID = gameObjectNode.attribute("UID").as_uint();
	enabled = gameObjectNode.attribute("Enabled").as_bool();
	isStatic = gameObjectNode.attribute("Static").as_bool();

	pugi::xml_node componentsNode = gameObjectNode.child("Components");

	if (!componentsNode.empty())
	{
		for (pugi::xml_node componentNode = componentsNode.child("Component"); componentNode; componentNode = componentNode.next_sibling("Component"))
		{
			ComponentType type = (ComponentType)componentNode.attribute("type").as_int();
			Component* component = GetComponent(type);
			if (!component)
				component = AddComponent(type);
			
			if(component) component->Load(componentNode);
			else
			{
				LOG("Failed to load component %d to %s game object", (int)type, name);
			}
		}
	}

	pugi::xml_node childsNode = gameObjectNode.child("Childs");
	if (!childsNode.empty())
	{
		for (pugi::xml_node childNode = childsNode.child("GameObject"); childNode; childNode = childNode.next_sibling("GameObject"))
		{
			GameObject* childObject = new GameObject(true, childNode.attribute("Name").as_string());

			if (childObject)
			{
				childObject->Load(childNode);
				AddChild(childObject);
			}
			else
			{
				LOG("Error: Could not create new GameObject while loading scene.");
			}
		}
	}
}

bool GameObject::TryGetGlobalMatrix(glm::mat4& globalMatrix)
{
	if (transform)
	{
		globalMatrix = transform->GetGlobalMatrix();
		return true;
	}
	else return false;

}
bool GameObject::TryGetGlobalAABB(AABB& globalAABB)
{
	Mesh* mesh = (Mesh*)GetComponent(ComponentType::Mesh);
	if (mesh && transform)
	{
		globalAABB = mesh->aabb->GetGlobalAABB(transform->GetGlobalMatrix());
		return true;
	}
	else return false;
}

void GameObject::SetStatic(bool _static)
{
	isStatic = _static;
	Engine::GetInstance().events->PublishImmediate(Event(Event::Type::StaticChanged, this));
}

void GameObject::SetEnabled(bool _enabled)
{
	if (enabled == _enabled) return;

	bool wasActive = GetEnabled();

	enabled = _enabled;

	bool nowActive = GetEnabled();

	if (wasActive != nowActive)
	{
		UpdateEnabledRecursive(nowActive);
	}
}

void GameObject::UpdateEnabledRecursive(bool effectiveState)
{
	if (effectiveState)
		OnEnable();
	else
		OnDisable();

	for (GameObject* child : childs)
	{
		if (child->enabled)
		{
			child->UpdateEnabledRecursive(effectiveState);
		}
	}
}

bool GameObject::GetEnabled()
{
	if (!enabled) return false;

	if (parent != nullptr)
	{
		return parent->GetEnabled();
	}

	return true;
}

bool GameObject::OnEnable()
{
	for (auto const& pair : components)
	{
		Component* component = pair.second;
		if (component->enabled)
		{
			component->OnEnable();
		}
	}

	return true;
}

bool GameObject::OnDisable()
{
	for (auto const& pair : components)
	{
		Component* component = pair.second;
		if (component->enabled)
		{
			component->OnDisable();
		}
	}

	return true;
}

bool GameObject::GetStatic()
{
	return isStatic;
}

void GameObject::RemoveChild(GameObject* childToRemove)
{
	auto it = std::find(childs.begin(), childs.end(), childToRemove);
	if (it != childs.end())
	{
		childs.erase(it);
	}
}

bool GameObject::IsDescendant(GameObject* potentialDescendant)
{
	for (GameObject* child : childs)
	{
		if (child == potentialDescendant) return true;

		if (child->IsDescendant(potentialDescendant)) return true;
	}
	return false;
}

void GameObject::SetParent(GameObject* newParent)
{
	if (parent == newParent) return;
	if (newParent == this) return;
	if (newParent != nullptr && IsDescendant(newParent)) return;

	glm::mat4 currentGlobalMatrix = transform->GetGlobalMatrix();
	bool wasActive = GetEnabled();

	if (parent != nullptr)
	{
		parent->RemoveChild(this);
	}
	else
	{
		Engine::GetInstance().scene->RemoveGameObject(this);
	}

	parent = newParent;
	parentUUID = (parent) ? parent->UUID : 0;

	if (parent != nullptr)
	{
		parent->childs.push_back(this);
	}
	else
	{
		Engine::GetInstance().scene->AddGameObject(this);
	}

	if (parent != nullptr)
	{
		glm::mat4 parentGlobal = parent->transform->GetGlobalMatrix();
		glm::mat4 parentInverse = glm::inverse(parentGlobal);
		glm::mat4 newLocal = parentInverse * currentGlobalMatrix;

		transform->SetLocalMatrix(newLocal);
	}
	else
	{
		transform->SetLocalMatrix(currentGlobalMatrix);
	}

	transform->GetGlobalMatrix();

	bool nowActive = GetEnabled();
	if (wasActive != nowActive)
	{
		UpdateEnabledRecursive(nowActive);
	}
}