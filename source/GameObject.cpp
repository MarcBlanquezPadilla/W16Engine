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
#include "components/Rigidbody.h"
#include "components/BoxCollider.h"
#include "components/SphereCollider.h"
#include "components/CapsuleCollider.h"
#include "components/DistanceJoint.h"
#include "components/FixedJoint.h"
#include "components/HingeJoint.h"
#include "components/PrismaticJoint.h"
#include "components/SphericalJoint.h"
#include "components/D6Joint.h"
#include "utils/Log.h"
#include "utils/AABB.h"
#include "utils/Config.h"
#include "imgui.h"

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

bool GameObject::FixedUpdate()
{
	bool ret = true;

	auto it = components.begin();
	while (it != components.end())
	{
		Component* component = it->second;

		if (component->GetEnabled())
		{
			component->FixedUpdate();
		}
		++it;
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
	for (auto const& pair : components)
	{
		if (pair.second->IsIncompatible(type))
		{
			LOG(LogType::LOG_ERROR, "Could not add component: Conflict with component %s", pair.second->name.c_str());
			return nullptr;
		}
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
	case ComponentType::Rigidbody:
		component = new Rigidbody(this);
		break;
	case ComponentType::BoxCollider:
		component = new BoxCollider(this);
		break;
	case ComponentType::SphereCollider:
		component = new SphereCollider(this);
		break;
	case ComponentType::CapsuleCollider:
		component = new CapsuleCollider(this);
		break;
	case ComponentType::DistanceJoint:
		component = new DistanceJoint(this);
		break;
	case ComponentType::FixedJoint:
		component = new FixedJoint(this);
		break;
	case ComponentType::HingeJoint:
		component = new HingeJoint(this);
		break;
	case ComponentType::PrismaticJoint:
		component = new PrismaticJoint(this);
		break;
	case ComponentType::SphericalJoint:
		component = new SphericalJoint(this);
		break;
	case ComponentType::D6Joint:
		component = new D6Joint(this);
		break;
	}

	if (component == nullptr) return nullptr;

	PublishGameObjectEvent(GameObjectEvent::COMPONENT_ADDED, component);
	components[type] = component;
	component->owner = this;
	component->Start();
	component->OnEnable();

	return component;
}

void GameObject::RemoveComponent(ComponentType type)
{
	if (components.count(type) == 0 || type == ComponentType::Transform) return;
	
	Component* componentToRemove = components[type];
	componentsToDestroy.push_back(componentToRemove);
	PublishGameObjectEvent(GameObjectEvent::COMPONENT_REMOVED, componentToRemove);
}

Component* GameObject::GetComponent(ComponentType type)
{
	for (auto& pair : components)
	{
		if (pair.second->IsType(type))
		{
			return pair.second;
		}
	}
	return nullptr;
}

Component* GameObject::GetComponentInChildren(ComponentType type)
{
	Component* component = GetComponent(type);

	if (component) return component;

	for (GameObject* child : childs)
	{
		if (child)
		{
			component = child->GetComponentInChildren(type);
			if (component) return component;
		}
	}

	return nullptr;
}

void GameObject::GetComponentsInChildren(ComponentType type, std::vector<Component*>& outList)
{
	Component* component = GetComponent(type);
	if (component) outList.push_back(component);

	for (GameObject* child : childs)
	{
		if (child) child->GetComponentsInChildren(type, outList);
	}
}

Component* GameObject::GetComponentInParent(ComponentType type)
{
	Component* component = GetComponent(type);
	if (component) return component;

	if (parent != nullptr)
	{
		return parent->GetComponentInParent(type);
	}

	return nullptr;
}

void GameObject::GetComponentsInParent(ComponentType type, std::vector<Component*>& outList)
{
	Component* component = GetComponent(type);
	if (component) outList.push_back(component);

	if (parent != nullptr)
	{
		parent->GetComponentsInParent(type, outList);
	}
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
				AddChild(childObject);
				childObject->Load(childNode);
			}
			else
			{
				LOG(LogType::LOG_ERROR, "Could not create new GameObject while loading scene.");
			}

			childNode = childNode.GetNextSibling("GameObject");
		}
	}
}

void GameObject::SolveReferences()
{
	for (auto const& pair : components)
	{
		Component* component = pair.second;
		if (component)
		{
			component->ResolveReferences();
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

void GameObject::OnEditor()
{
	char name_buffer[256];
	sprintf_s(name_buffer, "%s", name.c_str());

	if (ImGui::InputText("Name", name_buffer, sizeof(name_buffer)))
	{
		name = name_buffer;
	}

	bool isEnabled = GetEnabled();
	if (ImGui::Checkbox("Enabled", &isEnabled))
	{
		SetEnabled(isEnabled);
	}

	ImGui::SameLine();

	bool isStatic = GetStatic();
	if (ImGui::Checkbox("Static", &isStatic))
	{
		SetStatic(isStatic);
	}

	for (auto const& pair : components)
	{
		Component* comp = pair.second;
		if (!comp) continue;

		ImGui::PushID(comp);

		if (ImGui::CollapsingHeader(comp->name.c_str()))
		{
			ImGui::BeginGroup();

			ImGui::Indent(10.0f);
			ImGui::Spacing();

			comp->OnEditor();

			ImGui::Spacing();
			ImGui::Unindent(10.0f);

			ImGui::EndGroup();
		}
		if (ImGui::BeginPopupContextItem("ComponentOptions"))
		{
			if (ImGui::MenuItem("Remove Component")) {
				RemoveComponent(pair.first);
			}
			ImGui::EndPopup();
		}

		ImGui::PopID();
	}
	
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();

	float buttonWidth = ImGui::GetContentRegionAvail().x * 0.6f;
	float centerPos = (ImGui::GetContentRegionAvail().x - buttonWidth) * 0.5f;
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + centerPos);

	if (ImGui::Button("Add Component", ImVec2(buttonWidth, 0)))
	{
		ImGui::OpenPopup("AddComponentPopup");
	}

	if (ImGui::BeginPopup("AddComponentPopup"))
	{
		if (GetComponent(ComponentType::MeshRenderer) == nullptr)
		{
			if (ImGui::MenuItem("Mesh Renderer")) { AddComponent(ComponentType::MeshRenderer); ImGui::CloseCurrentPopup(); }
		}
		if (GetComponent(ComponentType::Animation) == nullptr)
		{
			if (ImGui::MenuItem("Animation")) { AddComponent(ComponentType::Animation); ImGui::CloseCurrentPopup(); }
		}
		if (GetComponent(ComponentType::Rigidbody) == nullptr)
		{
			if (ImGui::MenuItem("Rigidbody")) { AddComponent(ComponentType::Rigidbody); ImGui::CloseCurrentPopup(); }
		}
		if (GetComponent(ComponentType::Collider) == nullptr)
		{
			if (ImGui::MenuItem("Box Collider")) { AddComponent(ComponentType::BoxCollider); ImGui::CloseCurrentPopup(); }
		}
		if (GetComponent(ComponentType::Collider) == nullptr)
		{
			if (ImGui::MenuItem("Sphere Collider")) { AddComponent(ComponentType::SphereCollider); ImGui::CloseCurrentPopup(); }
		}
		if (GetComponent(ComponentType::Collider) == nullptr)
		{
			if (ImGui::MenuItem("Capsule Collider")) { AddComponent(ComponentType::CapsuleCollider); ImGui::CloseCurrentPopup(); }
		}
		if (GetComponent(ComponentType::Camera) == nullptr)
		{
			if (ImGui::MenuItem("Camera")) { AddComponent(ComponentType::Camera); ImGui::CloseCurrentPopup(); }
		}
		
		if (ImGui::MenuItem("Distance Joint")) { AddComponent(ComponentType::DistanceJoint); ImGui::CloseCurrentPopup(); }
		if (ImGui::MenuItem("Fixed Joint")) { AddComponent(ComponentType::FixedJoint); ImGui::CloseCurrentPopup(); }
		if (ImGui::MenuItem("Hinge Joint")) { AddComponent(ComponentType::HingeJoint); ImGui::CloseCurrentPopup(); }
		if (ImGui::MenuItem("Prismatic Joint")) { AddComponent(ComponentType::PrismaticJoint); ImGui::CloseCurrentPopup(); }
		if (ImGui::MenuItem("Shperical Joint")) { AddComponent(ComponentType::SphericalJoint); ImGui::CloseCurrentPopup(); }
		if (ImGui::MenuItem("D6 Joint")) { AddComponent(ComponentType::D6Joint); ImGui::CloseCurrentPopup(); }
		

		ImGui::EndPopup();
	}
}

void GameObject::PublishGameObjectEvent(GameObjectEvent event, Component* newComponent)
{
	auto it = components.begin();
	while (it != components.end())
	{
		Component* component = it->second;

		if (component->GetEnabled())
		{
			component->OnGameObjectEvent(event, newComponent);
		}
		++it;
	}
}