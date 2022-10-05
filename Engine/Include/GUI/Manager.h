#pragma once
#include "GFX/Graphics.h"
#include "Window/MainWindow.h"
#include "DearImGui.h"

namespace ZE::GUI
{
	// Interacting with GUI subsystem
	class Manager final
	{
	public:
		Manager();
		ZE_CLASS_DELETE(Manager);
		~Manager() { Disable(); ImGui::DestroyContext(); }

		void Init(GFX::Device& dev) const noexcept;
		void Disable() const noexcept;
		void StartFrame(const Window::MainWindow& window) const noexcept;
		void EndFrame(GFX::Graphics& gfx) const noexcept;

		void SetFont(const std::string& font, float size) const;
	};
}