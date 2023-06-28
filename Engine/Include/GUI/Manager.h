#pragma once
#include "GFX/Graphics.h"
#include "Window/MainWindow.h"
#include "DearImGui.h"

namespace ZE::GUI
{
	// Interacting with GUI subsystem
	class Manager final
	{
		Ptr<U8> backendData;

		void RebuildFontsVK(GFX::Device& dev, GFX::CommandList& cl) const;

	public:
		Manager();
		ZE_CLASS_DELETE(Manager);
		~Manager() { ZE_ASSERT(backendData == nullptr, "GUI backend data not freed!"); ImGui::DestroyContext(); }

		void Init(GFX::Graphics& gfx, bool backbufferSRV);
		void Destroy(GFX::Device& dev) noexcept;
		void StartFrame(const Window::MainWindow& window) const noexcept;
		void EndFrame(GFX::Graphics& gfx) const noexcept;

		void SetFont(GFX::Graphics& gfx, const std::string& font, float size) const;
	};
}