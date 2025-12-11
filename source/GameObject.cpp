#include "GameObject.h"
#include "ModuleScene.h"
#include "Engine.h"
#include "ModuleEvents.h"
#include "components/Component.h"
#include "components/Mesh.h"
#include "components/Transform.h"
#include "components/Texture.h"
#include "components/Camera.h"
#include "utils/Log.h"
#include "utils/AABB.h"
#include "utils/Config.h"

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
	parent = nullptr;
	transform = nullptr;
	childs.clear();
	components.clear();
	AddComponent(ComponentType::Transform);
}

GameObject::~GameObject()
{

}


bool GameObject::Update()
{
	bool ret = true;

	for (auto const& pair : components)
	{
		Component* component = pair.second;
		if (component->enabled)
		{
			component->Update();
		}
	}

	for (auto child : childs)
	{
		child->Update();
	}

	return ret;
}

bool GameObject::CleanUp()
{
	Engine::GetInstance().moduleEvents->PublishImmediate(Event(Event::Type::GameObjectDestroyed, this));

	for (auto const& pair : components)
	{
		pair.second->CleanUp();
		delete pair.second;
	}
	components.clear();

	for (GameObject* child : childs)
	{
		if (child != nullptr) {
			child->parent = nullptr;
		}
	}

	childs.clear();

	return true;
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
	gameObject->parent = this;
	childs.push_back(gameObject);
}

void GameObject::Save(Config& gameObjectNode)
{
	gameObjectNode.SetString("Name", name.c_str());
	gameObjectNode.SetUInt("UID", UUID);
	gameObjectNode.SetBool("Enabled", enabled);
	gameObjectNode.SetBool("Static", isStatic);

	if (components.size() > 0)
	{
		Config componentsNode = gameObjectNode.AddChild("Components");
		for (auto const& pair : components)
		{
			Config compoenntNode = componentsNode.AddChild("Component");
			Component* component = pair.second;
			if (component)
			{
				compoenntNode.SetInt("type", (int)component->GetType());
				compoenntNode.SetBool("enabled", component->enabled);
				component->Save(compoenntNode);
			}
		}
	}

	if (childs.size() > 0)
	{
		Config childsNode = gameObjectNode.AddChild("Childs");
		for (GameObject* child : childs)
		{
			Config childNode = childsNode.AddChild("GameObject");
			child->Save(childNode);
		}
	}
}

void GameObject::Load(Config& gameObjectNode)
{
	name = gameObjectNode.GetString("Name");
	UUID = gameObjectNode.GetUInt("UID");
	enabled = gameObjectNode.GetBool("Enabled");
	isStatic = gameObjectNode.GetBool("Static");

	//LOAD COMPONENTS
	Config componentsNode = gameObjectNode.GetChild("Components");

	if (componentsNode.IsValid())
	{
		Config componentNode = componentsNode.GetChild("Component");
		while (componentNode.IsValid())
		{
			ComponentType type = (ComponentType)componentNode.GetInt("type");
			Component* component = GetComponent(type);
			if (!component)
				component = AddComponent(type);

			if (component) component->Load(componentNode);
			else
			{
				LOG("Failed to load component %d to %s game object", (int)type, name);
			}

			componentNode = componentNode.GetNextSibling("Component");
		}
	}

	//LOAD CHILDS
	Config childsNode = gameObjectNode.GetChild("Childs");

	if (childsNode.IsValid())
	{
		Config childNode = childsNode.GetChild("GameObject");
		
		while (childNode.IsValid())
		{
			GameObject* childObject = new GameObject(true, childNode.GetString("Name"));

			if (childObject)
			{
				childObject->Load(childNode);
				AddChild(childObject);
			}
			else
			{
				LOG("Error: Could not create new GameObject while loading scene.");
			}

			childNode = childNode.GetNextSibling("GameObject");
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
		globalAABB = mesh->GetGlobalAABB();
		return true;
	}
	else return false;
}

void GameObject::SetStatic(bool _static)
{
	isStatic = _static;
	Engine::GetInstance().moduleEvents->PublishImmediate(Event(Event::Type::StaticChanged, this));
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
	if (childToRemove == nullptr) return;

	auto it = std::remove(childs.begin(), childs.end(), childToRemove);

	if (it != childs.end())
	{
		childs.erase(it, childs.end());

		childToRemove->parent = nullptr;

		Transform* childTransform = (Transform*)childToRemove->GetComponent(ComponentType::Transform);
		if (childTransform)
		{
			glm::mat4 globalMatrix = childTransform->GetGlobalMatrix();
			childTransform->SetLocalMatrix(globalMatrix);
		}
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
		Engine::GetInstance().moduleScene->RemoveGameObject(this);
	}

	parent = newParent;

	if (parent != nullptr)
	{
		parent->childs.push_back(this);
	}
	else
	{
		Engine::GetInstance().moduleScene->AddGameObject(this);
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

void GameObject::Destroy()
{
	Engine::GetInstance().moduleScene->DestroyGameObject(this);
}