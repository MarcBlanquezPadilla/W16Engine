#pragma once
#include "UIWindow.h"
#include <vector>
#include <windows.h>
#include <psapi.h>

class ConfigWindow : public UIWindow
{
public:
    ConfigWindow();
    virtual ~ConfigWindow();

    void Draw() override;

private:
    std::vector<float> fps_log;
    std::vector<float> memory_log;

    PROCESS_MEMORY_COUNTERS mem_counters;
};