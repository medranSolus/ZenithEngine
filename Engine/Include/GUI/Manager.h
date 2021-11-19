#pragma once
#include "Window/MainWindow.h"
#include "GFX/Graphics.h"
#include "imgui.h"
#include <string>

namespace ZE::GUI
{
	// Interacting with GUI subsystem
	class Manager final
	{
	public:
		Manager();
		Manager(Manager&&) = delete;
		Manager(const Manager&) = delete;
		Manager& operator=(Manager&&) = delete;
		Manager& operator=(const Manager&) = delete;
		~Manager() { Disable(); ImGui::DestroyContext(); }

		void Init(GFX::Device& dev) const noexcept;
		void Disable() const noexcept;
		void StartFrame(const Window::MainWindow& window) const noexcept;
		void EndFrame(GFX::Graphics& gfx) const noexcept;

		void SetFont(const std::string& font, float size) const;
	};
}