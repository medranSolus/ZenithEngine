#pragma once
#include "StartupConfig.h"
#include "GUI/Manager.h"
#include "GFX/Pipeline/RendererPBR.h"

namespace ZE
{
	// Main Zenith Engine component containing all the rendering logic
	class Engine final : public StartupConfig
	{
		GUI::Manager gui;
		Window::MainWindow window;
		GFX::Graphics graphics;
		GFX::Pipeline::RendererPBR renderer;
		GFX::Resource::Texture::Library textureLib;
		bool guiEnabled = true;

	public:
		Engine(const EngineParams& params);
		ZE_CLASS_DELETE(Engine);
		virtual ~Engine();

		constexpr bool IsGuiActive() const noexcept { return guiEnabled; }
		constexpr void ToggleGui() noexcept { guiEnabled = !guiEnabled; }

		constexpr Data::Storage& GetData() noexcept { return renderer.GetRegistry(); }
		constexpr Data::Storage& GetResourceData() noexcept { return renderer.GetResources(); }
		constexpr GFX::Resource::Texture::Library& GetTextureLibrary() noexcept { return textureLib; }
		constexpr Window::MainWindow& Window() noexcept { return window; }
		constexpr GFX::Graphics& Gfx() noexcept { return graphics; }
		constexpr GFX::Pipeline::RendererPBR& Reneder() noexcept { return renderer; }

		void BeginFrame();
		void EndFrame();
	};
}