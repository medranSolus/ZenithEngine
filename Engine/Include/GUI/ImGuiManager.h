#pragma once
#include "Window/MainWindow.h"
#include "DearImGui.h"

namespace ZE::GUI
{
	// Interacting with ImGui subsystem
	class ImGuiManager final
	{
	public:
		ImGuiManager() noexcept;
		ZE_CLASS_DELETE(ImGuiManager);
		~ImGuiManager() { ImGui::DestroyContext(); }

		void EndFrame() const noexcept { ImGui::Render(); }

		void StartFrame(const Window::MainWindow& window) const noexcept;
		void SetFont(std::string_view font, float size) const noexcept;
	};
}