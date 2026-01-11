#pragma once
#include "UIWindow.h"
#include <vector>
#include <windows.h>
#include <psapi.h>

class ConsoleWindow : public UIWindow
{
public:
    ConsoleWindow(bool active);
    virtual ~ConsoleWindow();

    void Draw() override;

private:
    bool info = true;
    bool warning = true;
    bool error = true;
};