#include "Camera.h"
#include "../GameObject.h"
#include "../CameraLens.h"
#include "../components/Transform.h"
#include "../Engine.h"
#include "../EventSystem.h"
#include "../Render.h"

#include "imgui.h"

Camera::Camera(GameObject* owner) : Component(owner)
{

}

Camera::~Camera()
{

}

void Camera::Start()
{
    lens = new CameraLens();
    lens->SetPerspective(60.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    Engine::GetInstance().events->Subscribe(Event::Type::ChangeActiveCamera, this);
}

void Camera::OnEnable()
{
    Engine::GetInstance().render->AddCamera(lens);
}

void Camera::OnDisable()
{
    Engine::GetInstance().render->RemoveCamera(lens);
}

void Camera::Update(float dt)
{
    Transform* transform = (Transform*)owner->GetComponent(ComponentType::Transform);

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
    Engine::GetInstance().events->PublishImmediate(Event(Event::Type::ChangeActiveCamera, owner->UUID));
}

void Camera::CleanUp()
{
    Engine::GetInstance().render->RemoveCamera(lens);
    lens->CleanUp();
    delete lens;
}

void Camera::OnEditor()
{
    if (ImGui::CollapsingHeader("Camera"))
    {
        if (ImGui::DragFloat("FOV", &lens->fov, 0.1f, 1.0f, 179.0f));

        if (ImGui::DragFloat("Near Plane", &lens->zNear, 0.1f, 0.01f, 1000.0f));

        if (ImGui::DragFloat("Far Plane", &lens->zFar, 1.0f, 0.1f, 10000.0f));

        bool isMain = isMainCamera;
        if (ImGui::Checkbox("Main Camera", &isMain))
        {
            SetMainCamera(isMain);
        }

        ImGui::Text("FBO:");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.9f, 1.0f), "%dx", lens->fboID);
    }
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