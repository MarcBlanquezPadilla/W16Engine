#pragma once
#include "UIWindow.h"
#include "../EventListener.h"
#include <vector>

class GameObject;

class InspectorWindow : public UIWindow, public EventListener
{
public:
    InspectorWindow(bool active);
    virtual ~InspectorWindow();

    void CleanUp() override;

    void Draw() override;
    void OnEvent(const Event& event) override;

private:
    bool inspectorLocked = false;
    std::vector<GameObject*> lockedObjects;
};