#pragma once
#include "UIWindow.h"

class ProjectWindow : public UIWindow
{
public:
    ProjectWindow(bool active);
    virtual ~ProjectWindow();

    void Draw() override;

private:

};