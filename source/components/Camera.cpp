#include "Camera.h"
#include "../GameObject.h"
#include "../CameraLens.h"
#include "../components/Transform.h"
#include "../Engine.h"
#include "../ModuleEvents.h"
#include "../ModuleRender.h"

#include "imgui.h"

Camera::Camera(GameObject* owner) : Component(owner)
{
    name = "Camera";
}

Camera::~Camera()
{

}

void Camera::Start()
{
    lens = new CameraLens();
    
    UpdateTransform();

    lens->SetPerspective(60.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    Engine::GetInstance().moduleEvents->Subscribe(Event::Type::ChangeActiveCamera, this);
}

void Camera::OnEnable()
{
    Engine::GetInstance().moduleRender->AddCamera(lens);
}

void Camera::OnDisable()
{
    Engine::GetInstance().moduleRender->RemoveCamera(lens);
}

void Camera::Update()
{
    UpdateTransform();
    lens->SetActiveCamera(GetEnabled());
}

void Camera::UpdateTransform()
{
    Transform* transform = (Transform*)owner->transform;

    if (transform && lens)
    {
        glm::mat4 modelMatrix = transform->GetGlobalMatrix();
        glm::mat4 viewMatrix = glm::inverse(modelMatrix);

        lens->SetViewMatrix(viewMatrix);
    }
}

void Camera::SetMainCamera(bool mainCamera)
{
    if (!owner->GetEnabled()) return;
    Engine::GetInstance().moduleEvents->PublishImmediate(Event(Event::Type::ChangeActiveCamera, owner->UUID));
}

void Camera::CleanUp()
{
    Engine::GetInstance().moduleEvents->UnsubscribeAll(this);
    Engine::GetInstance().moduleRender->RemoveCamera(lens);
    lens->CleanUp();
    delete lens;
}

void Camera::OnEditor()
{
    float fov = lens->GetFov();
    if (ImGui::DragFloat("FOV", &fov, 0.1f, 1.0f, 160.0f))
    {
        lens->SetFov(fov);
    }

    float zNear = lens->GetNearPlane();
    if (ImGui::DragFloat("Near Plane", &zNear, 0.1f, 0.01f, 1000.0f))
    {
        lens->SetNearPlane(zNear);
    }

    float zFar = lens->GetFarPlane();
    if (ImGui::DragFloat("Far Plane", &zFar, 1.0f, 0.1f, 10000.0f))
    {
        lens->SetFarPlane(zFar);
    }

    int depth = lens->depth;
    if (ImGui::InputInt("Depth", &depth))
    {
        lens->depth = glm::clamp(depth, 0, 100000);
    }

    ImGui::Text("FBO:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.9f, 1.0f), "%dx", lens->fboID);

    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    viewportSize.y = viewportSize.x / lens->GetAspectRatio();
    ImVec2 winPos = ImGui::GetCursorScreenPos();
    unsigned int textureID = lens->textureID;
        
    ImGui::Separator();
    ImGui::Image((ImTextureID)(intptr_t)textureID, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
}

void Camera::OnEvent(const Event& event)
{
    switch (event.type)
    {
    case Event::Type::ChangeActiveCamera:
    {
        unsigned int incomingUUID = event.data.unsignedInt.unsignedInt;

        if (owner->UUID == incomingUUID)
        {
            isMainCamera = !isMainCamera;
        }
        else
        {
            isMainCamera = false;
        }
        break;
    }
    
    default:
        break;
    }
}

void Camera::Save(Config& componentNode)
{
    componentNode.SetFloat("fov", lens->GetFov());
    componentNode.SetFloat("farPlane", lens->GetFarPlane());
    componentNode.SetFloat("nearPlane", lens->GetNearPlane());
    componentNode.SetInt("depth", lens->depth);
}

void Camera::Load(Config& componentNode)
{
    lens->SetFov(componentNode.GetFloat("fov"));
    lens->SetFarPlane(componentNode.GetFloat("farPlane"));
    lens->SetNearPlane(componentNode.GetFloat("nearPlane"));
    lens->depth = componentNode.GetInt("depth");
}