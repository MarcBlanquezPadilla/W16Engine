#pragma once
#include "UIWindow.h"

class HierarchyWindow : public UIWindow
{
public:
    HierarchyWindow(bool active);
    virtual ~HierarchyWindow();

    void Draw() override;

private:

};