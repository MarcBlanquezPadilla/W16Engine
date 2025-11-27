#pragma once
#include "UIWindow.h"
#include <string>

class GameWindow : public UIWindow
{
public:
    GameWindow(bool active);
    virtual ~GameWindow();

    void Draw() override;

private:

};