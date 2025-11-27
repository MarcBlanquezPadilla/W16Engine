#include "UIWindow.h"
#include "imgui.h"

bool UIWindow::IsHovered()
{
	return ImGui::IsWindowHovered();
}