#pragma once
#include "UIWindow.h"

class GameObject;

class HierarchyWindow : public UIWindow
{
public:
    HierarchyWindow(bool active);
    virtual ~HierarchyWindow();

    void Draw() override;
    void DrawGameObjectNode(GameObject* go);

public:
    bool isFocused;

private:
    GameObject* objectToDrop = nullptr;
    GameObject* objectToSelect = nullptr;
    bool dragging = false;
};