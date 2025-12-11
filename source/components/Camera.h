#pragma once
#include "Component.h"
#include "../EventListener.h"

class GameObject;
class CameraLens;

class Camera : public Component, public EventListener
{
public:
    Camera(GameObject* owner);
    ~Camera();

    ComponentType GetType() override {
        return ComponentType::Camera;
    };

    void Start() override;
    void OnEnable() override;
    void OnDisable() override;

    void Update() override;
    void UpdateTransform();
    
    void CleanUp() override;

    void OnEditor() override;
    void Save(Config& componentNode) override;
    void Load(Config& componentNode) override;

    CameraLens* GetLens() { return lens; }

    void SetMainCamera(bool mainCamera);

    //EVENTS
    void OnEvent(const Event& event) override;

    bool isMainCamera = false;

private:
    CameraLens* lens = nullptr;
};