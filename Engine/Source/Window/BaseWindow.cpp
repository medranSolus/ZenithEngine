#include "Window/BaseWindow.h"

namespace ZE::Window
{
	void BaseWindow::EnableCursor() noexcept
	{
		if (!IsCursorEnabled())
		{
			flags[0] = true;
			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
			ShowCursor();
			FreeCursor();
		}
	}

	void BaseWindow::DisableCursor() noexcept
	{
		if (IsCursorEnabled())
		{
			flags[0] = false;
			ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
			HideCursor();
			TrapCursor();
		}
	}

	void BaseWindow::SwitchFullscreen() noexcept
	{
		if (IsFullscreen())
		{
			flags[1] = false;
			LeaveFullscreen();
		}
		else
		{
			flags[1] = true;
			EnterFullscreen();
		}
	}
}