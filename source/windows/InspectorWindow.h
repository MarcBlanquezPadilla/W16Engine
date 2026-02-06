#pragma once
#include "UIWindow.h"

class GameObject;

class InspectorWindow : public UIWindow
{
public:
    InspectorWindow(bool active);
    virtual ~InspectorWindow();

    void Draw() override;
};