#pragma once
#include "CommandList.h"
#include "Device.h"
#include "SwapChain.h"

namespace ZE::GFX
{
	class Graphics final
	{
		Device device;
		CommandList mainList;
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
		constexpr CommandList& GetMainList() noexcept { return mainList; }
		constexpr void Present() { swapChain.Present(device); }

		void Init(const Window::MainWindow& window);
	};
}