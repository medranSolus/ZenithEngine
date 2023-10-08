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

		double prevTime = 0.0;
		GFX::Graphics graphics;
		GUI::Manager gui;
		Window::MainWindow window;
		GFX::Pipeline::RendererPBR renderer;
		Data::ResourceManager resources;
		std::bitset<Flags::Count> flags;

	public:
		Engine(const SettingsInitParams& params) noexcept : StartupConfig(params) {}
		ZE_CLASS_DELETE(Engine);
		virtual ~Engine();

		constexpr bool IsGuiActive() const noexcept { return flags[Flags::GuiEnable]; }
		constexpr void ToggleGui() noexcept { SetGui(!IsGuiActive()); }
		constexpr void SetGui(bool enabled) noexcept { flags[Flags::GuiEnable] = enabled; }

		constexpr Data::AssetsStreamer& GetAssetsStreamer() noexcept { return renderer.GetAssetsStreamer(); }
		constexpr GFX::Graphics& Gfx() noexcept { return graphics; }
		constexpr GUI::Manager& Gui() noexcept { return gui; }
		constexpr Window::MainWindow& Window() noexcept { return window; }
		constexpr GFX::Pipeline::RendererPBR& Reneder() noexcept { return renderer; }
		constexpr Data::ResourceManager& Resources() noexcept { return resources; }

		// Initialization method that must be called before using engine
		void Init(const EngineParams& params);
		// Need to be called before starting first frame
		void Start(EID camera) noexcept;
		// Returns number of update steps that have to be taken by simulations multiplied by delta time
		double BeginFrame(double deltaTime, U64 maxUpdateSteps);
		void EndFrame();
	};
}