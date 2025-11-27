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

private: 
    void DrawHierarchyNode(GameObject* gameObject);

private:
    GameObject* draggedGameObject = nullptr;
    GameObject* targetGameObject = nullptr;
    bool toRoot = false;
};