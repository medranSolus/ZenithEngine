#include "Window/MainWindow.h"
#include "imgui.h"

namespace ZE::Window
{
	void BaseWindow::EnableCursor() noexcept
	{
		if (!cursorEnabled)
		{
			cursorEnabled = true;
			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
			ShowCursor();
			FreeCursor();
		}
	}

	void BaseWindow::DisableCursor() noexcept
	{
		if (cursorEnabled)
		{
			cursorEnabled = false;
			ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
			HideCursor();
			TrapCursor();
		}
	}
}