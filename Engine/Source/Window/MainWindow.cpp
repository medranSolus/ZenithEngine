#include "Window/MainWindow.h"
#include <filesystem>

namespace ZE::Window
{
	BaseWindow::BaseWindow(const char* name, U32 width, U32 height) noexcept
	{
		flags[0] = true;
		flags[1] = false;

		if (!std::filesystem::exists("imgui.ini") && std::filesystem::exists("imgui_default.ini"))
			std::filesystem::copy_file("imgui_default.ini", "imgui.ini");

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		auto& style = ImGui::GetStyle();
		style.WindowRounding = 1;
		style.WindowBorderSize = 1;
		style.Colors[ImGuiCol_WindowBg].w = 0.785f;
	}

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

	void BaseWindow::SetGuiFont(const std::string& font, float size) const
	{
		ImFontAtlas* atlas = ImGui::GetIO().Fonts;
		if (atlas->Fonts.size())
			atlas->Clear();
		atlas->AddFontFromFileTTF(font.c_str(), size);
	}
}