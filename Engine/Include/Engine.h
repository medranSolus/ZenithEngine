#pragma once
#include "GFX/Pipeline/RendererPBR.h"
#include "GUI/Manager.h"
#include "StartupConfig.h"

namespace ZE
{
	// Main Zenith Engine component containing all the rendering logic
	class Engine final : public StartupConfig
	{
		GFX::Graphics graphics;
		GUI::Manager gui;
		Window::MainWindow window;
		GFX::Pipeline::RendererPBR renderer;
		bool guiEnabled = true;

	public:
		Engine(const EngineParams& params);
		ZE_CLASS_DELETE(Engine);
		virtual ~Engine();

		constexpr bool IsGuiActive() const noexcept { return guiEnabled; }
		constexpr void ToggleGui() noexcept { guiEnabled = !guiEnabled; }
		constexpr void SetGui(bool enabled) noexcept { guiEnabled = enabled; }

		constexpr Data::Storage& GetData() noexcept { return renderer.GetRegistry(); }
		constexpr Data::AssetsStreamer& GetAssetsStreamer() noexcept { return renderer.GetAssetsStreamer(); }
		constexpr GFX::Graphics& Gfx() noexcept { return graphics; }
		constexpr GUI::Manager& Gui() noexcept { return gui; }
		constexpr Window::MainWindow& Window() noexcept { return window; }
		constexpr GFX::Pipeline::RendererPBR& Reneder() noexcept { return renderer; }

		void BeginFrame();
		void EndFrame();
	};
}