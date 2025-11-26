#pragma once
#include "UIWindow.h"
#include <string>

class SceneWindow : public UIWindow
{
public:
    SceneWindow(bool active);
    virtual ~SceneWindow();

    void Draw() override;

private:

};