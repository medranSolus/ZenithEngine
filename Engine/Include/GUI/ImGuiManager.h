#pragma once
#include "GFX/Graphics.h"
#include "Window/MainWindow.h"
#include "DearImGui.h"

namespace ZE::GUI
{
	// Data for render pass to use for running gpu job
	typedef Ptr<U8> ImGuiRenderData;

	// Interacting with ImGui subsystem
	class ImGuiManager final
	{
	public:
		ImGuiManager();
		ZE_CLASS_DELETE(ImGuiManager);
		~ImGuiManager() { ImGui::DestroyContext(); }

		// Function is static only for the sake of running it inside render pass
		static ImGuiRenderData CreateRenderData(GFX::Device& dev, PixelFormat outputFormat);
		static void DestroyRenderData(GFX::Device& dev, ImGuiRenderData& data) noexcept;
		static void RunRender(GFX::CommandList& cl) noexcept;

		void StartFrame(const Window::MainWindow& window) const noexcept;
		void EndFrame() const noexcept;
		void SetFont(std::string_view font, float size) const;
	};
}