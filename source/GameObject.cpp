#include "GameObject.h"
#include "ModuleScene.h"
#include "Engine.h"
#include "ModuleEvents.h"
#include "components/Component.h"
#include "components/MeshRenderer.h"
#include "components/SkinnedMeshRenderer.h"
#include "components/Transform.h"
#include "components/Camera.h"
#include "components/Animation.h"
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
	isStatic = false;
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

	auto it = components.begin();
	while (it != components.end())
	{
		Component* component = it->second;

		if (component->GetEnabled())
		{
			component->Update();
		}
		++it;
	}

	DeletePendingComponents();

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
	childs.clear();

	return true;
}

bool GameObject::CleanUpRecursive()
{
	for (auto const& pair : components)
	{
		pair.second->CleanUp();
		delete pair.second;
	}
	components.clear();

	for (GameObject* child : childs)
	{
		child->CleanUpRecursive();
		delete child;
		child = nullptr;
	}

	childs.clear();

	return true;
}

Component* GameObject::AddComponent(ComponentType type)
{
	if (components.count(type) > 0)
	{
		LOG(LogType::LOG_ERROR, "This GameObject already has a component of this type.");
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
	case ComponentType::MeshRenderer:
		component = new MeshRenderer(this);
		break;
	case ComponentType::SkinnedMeshRenderer:
		component = new SkinnedMeshRenderer(this);
		break;
	case ComponentType::Camera:
		component = new Camera(this);
		break;
	case ComponentType::Animation:
		component = new Animation(this);
		break;
	}

	if (component != nullptr)
	{
		components[type] = component;
		component->owner = this;
		component->Start();
		component->OnEnable();
	}
	return component;
}

void GameObject::RemoveComponent(ComponentType type)
{
	if (components.count(type) == 0 || type == ComponentType::Transform) return;
	
	Component* componentToRemove = components[type];
	componentsToDestroy.push_back(componentToRemove);
}

Component* GameObject::GetComponent(ComponentType type)
{
	for (auto pair : components)
	{
		if (pair.second->IsType(type))
		{
			return pair.second;
		}
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

void GameObject::DeletePendingComponents()
{
	if (componentsToDestroy.empty()) return;

	for (Component* component : componentsToDestroy)
	{
		components.erase(component->GetType());
		component->OnDisable();
		component->CleanUp();
		delete component;
		component = nullptr;
	}

	componentsToDestroy.clear();
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


GameObject* GameObject::FindChild(const std::string& nameToFind)
{
	if (name == nameToFind)
	{
		return this;
	}

	for (GameObject* child : childs)
	{
		GameObject* found = child->FindChild(nameToFind);

		if (found != nullptr)
		{
			return found;
		}
	}

	return nullptr;
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
				compoenntNode.SetBool("enabled", component->GetEnabled());
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
	if (UUID == 0) UUID = GenerateNewUID();
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
				LOG(LogType::LOG_ERROR, "Failed to load component %d to %s game object", (int)type, name);
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
				LOG(LogType::LOG_ERROR, "Could not create new GameObject while loading scene.");
			}

			childNode = childNode.GetNextSibling("GameObject");
		}
	}
}

bool GameObject::GetGlobalMatrix(glm::mat4& globalMatrix)
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
	MeshRenderer* mesh = (MeshRenderer*)GetComponent(ComponentType::MeshRenderer);
	if (mesh && transform)
	{
		globalAABB = mesh->GetGlobalAABB();
		return true;
	}
	else return false;
}

void GameObject::SetStatic(bool _static)
{
	if (isStatic == _static) return;
	isStatic = _static;
	Engine::GetInstance().moduleEvents->PublishImmediate(Event(Event::Type::StaticChanged, this));
}

void GameObject::SetEnabled(bool _enabled)
{
	if (enabled == _enabled) return;

	enabled = _enabled;
	UpdateEnabledRecursive(enabled);
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

const bool& GameObject::GetEnabled()
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
		if (!component->GetEnabled())
		{
			component->SetEnabled(true);
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
		if (component->GetEnabled())
		{
			component->SetEnabled(false);
			component->OnDisable();
		}
	}

	return true;
}

const bool& GameObject::GetStatic()
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

		Transform* childTransform = (Transform*)childToRemove->transform;
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
	if (parent == newParent) 
		return;
	if (newParent == this) 
		return;
	if (newParent != nullptr && IsDescendant(newParent)) 
		return;

	glm::mat4 currentGlobalMatrix = transform->GetGlobalMatrix();
	bool wasActive = GetEnabled();

	if (parent != nullptr)
	{
		parent->RemoveChild(this);
	}
	else
	{
		Engine::GetInstance().moduleScene->RemoveFromRoots(this);
	}

	parent = newParent;

	if (parent != nullptr)
	{
		parent->childs.push_back(this);
	}
	else
	{
		Engine::GetInstance().moduleScene->AddToRoots(this);
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