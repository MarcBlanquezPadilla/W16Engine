#pragma once

class UIWindow
{
public:
    UIWindow(const char* name, bool active) : name(name), is_active(active), defaultEnabled(is_active){}

    virtual ~UIWindow() {}

    virtual void CleanUp() {}

    virtual void Draw() = 0;

public:
    const char* name;
    bool is_active;
    bool defaultEnabled;
};