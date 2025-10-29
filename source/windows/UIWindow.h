#pragma once

class UIWindow
{
public:
    UIWindow(const char* name) : name(name), is_active(true) {}

    virtual ~UIWindow() {}

    virtual void Draw() = 0;

public:
    const char* name;
    bool is_active;
};