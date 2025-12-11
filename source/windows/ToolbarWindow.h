#pragma once
#include "UIWindow.h"
#include <string>

class ToolbarWindow : public UIWindow
{
public:
    ToolbarWindow(bool active);
    virtual ~ToolbarWindow();

    void Draw() override;

private:
    bool pauseOnPlay;
};