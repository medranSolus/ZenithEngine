#pragma once
#include "EngineParams.h"
#include "GUI/Manager.h"
#include "GFX/Pipeline/RendererPBR.h"

namespace ZE
{
	// Main Zenith Engine component containing all the rendering logic
	class Engine final
	{
		GUI::Manager gui;
		Window::MainWindow window;
		GFX::Graphics graphics;
		GFX::Pipeline::RendererPBR renderer;
		GFX::Resource::TextureLibrary textureLib;
		bool guiEnabled = true;

	public:
		Engine(const EngineParams& params);
		ZE_CLASS_DELETE(Engine);
		~Engine();

		constexpr bool IsGuiActive() const noexcept { return guiEnabled; }
		constexpr void ToggleGui() noexcept { guiEnabled = !guiEnabled; }

		constexpr Window::MainWindow& Window() noexcept { return window; }
		constexpr GFX::Graphics& Gfx() noexcept { return graphics; }

		void BeginFrame();
		void EndFrame();
	};
}