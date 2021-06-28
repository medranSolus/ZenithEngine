#pragma once
#include "GFX/Graphics.h"

namespace ZE
{
	class Engine final
	{
		Window::MainWindow window;
		GFX::Graphics graphics;
		bool guiEnabled = true;

	public:
		Engine(const char* windowName, GFX::API::Backend gfxApi, U32 width = 0, U32 height = 0);
		Engine(Engine&&) = delete;
		Engine(const Engine&) = delete;
		Engine& operator=(Engine&&) = delete;
		Engine& operator=(const Engine&) = delete;
		~Engine();

		constexpr bool IsGuiActive() const noexcept { return guiEnabled; }
		constexpr void ToggleGui() noexcept { guiEnabled = !guiEnabled; }

		constexpr Window::MainWindow& Window() noexcept { return window; }
		constexpr GFX::Graphics& Gfx() noexcept { return graphics; }

		void BeginFrame() const;
		void EndFrame() const;
	};
}