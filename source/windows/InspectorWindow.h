#pragma once
#include "UIWindow.h"

class InspectorWindow : public UIWindow
{
public:
    InspectorWindow(bool active);
    virtual ~InspectorWindow();

    void Draw() override;

private:

};