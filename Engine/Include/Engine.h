#pragma once
#include "GFX/Pipeline/RendererPBR.h"
#include "GUI/Manager.h"
#include "StartupConfig.h"

namespace ZE
{
	// Main Zenith Engine component containing all the rendering logic
	class Engine final : public StartupConfig
	{
		enum Flags : U8 { Initialized, GuiEnable, Count };

		GFX::Graphics graphics;
		GUI::Manager gui;
		Window::MainWindow window;
		GFX::Pipeline::RendererPBR renderer;
		std::bitset<Flags::Count> flags;

	public:
		Engine(const SettingsInitParams& params) noexcept : StartupConfig(params) {}
		ZE_CLASS_DELETE(Engine);
		virtual ~Engine();

		constexpr bool IsGuiActive() const noexcept { return flags[Flags::GuiEnable]; }
		constexpr void ToggleGui() noexcept { SetGui(!IsGuiActive()); }
		constexpr void SetGui(bool enabled) noexcept { flags[Flags::GuiEnable] = enabled; }

		constexpr Data::Storage& GetData() noexcept { return renderer.GetRegistry(); }
		constexpr Data::AssetsStreamer& GetAssetsStreamer() noexcept { return renderer.GetAssetsStreamer(); }
		constexpr GFX::Graphics& Gfx() noexcept { return graphics; }
		constexpr GUI::Manager& Gui() noexcept { return gui; }
		constexpr Window::MainWindow& Window() noexcept { return window; }
		constexpr GFX::Pipeline::RendererPBR& Reneder() noexcept { return renderer; }

		// Initialization method that must be called before using engine
		void Init(const EngineParams& params);
		void BeginFrame();
		void EndFrame();
	};
}