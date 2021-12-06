#pragma once
#include "EngineParams.h"
#include "GUI/Manager.h"
#include "GFX/Pipeline/RendererPBR.h"

namespace ZE
{
	// Main Zenith Engine component containing all rendering logic
	class Engine final
	{
		GUI::Manager gui;
		Window::MainWindow window;
		GFX::Graphics graphics;
		GFX::Pipeline::RendererPBR renderer;
		bool guiEnabled = true;

	public:
		Engine(const EngineParams& params);
		Engine(Engine&&) = delete;
		Engine(const Engine&) = delete;
		Engine& operator=(Engine&&) = delete;
		Engine& operator=(const Engine&) = delete;
		~Engine() = default;

		constexpr bool IsGuiActive() const noexcept { return guiEnabled; }
		constexpr void ToggleGui() noexcept { guiEnabled = !guiEnabled; }

		constexpr Window::MainWindow& Window() noexcept { return window; }
		constexpr GFX::Graphics& Gfx() noexcept { return graphics; }

		void BeginFrame();
		void EndFrame();
	};
}