#pragma once
#include "UIWindow.h"


class AboutWindow : public UIWindow
{
public:
    AboutWindow(bool active);
    virtual ~AboutWindow();

    void Draw() override;

private:

};