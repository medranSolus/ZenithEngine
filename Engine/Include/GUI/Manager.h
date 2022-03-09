#pragma once
#include "Window/MainWindow.h"
#include "GFX/Graphics.h"
#include "WarningGuardOn.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "WarningGuardOff.h"
#include <string>

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