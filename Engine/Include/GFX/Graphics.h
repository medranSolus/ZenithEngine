#pragma once
#include "Context.h"
#include "Device.h"
#include "SwapChain.h"

namespace ZE::GFX
{
	class Graphics final
	{
		Device device;
		Context mainCtx;
		SwapChain swapChain;
		bool guiEnabled = true;

	public:
		Graphics() = default;
		Graphics(Graphics&&) = delete;
		Graphics(const Graphics&) = delete;
		Graphics& operator=(Graphics&&) = delete;
		Graphics& operator=(const Graphics&) = delete;
		~Graphics() = default;

		constexpr Device& GetDevice() noexcept { return device; }
		constexpr Context& GetMainContext() noexcept { return mainCtx; }
		constexpr void Present() { swapChain.Present(device); }

		void Init(const Window::MainWindow& window);
	};
}