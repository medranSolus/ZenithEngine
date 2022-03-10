#pragma once
#include "CommandList.h"
#include "Device.h"
#include "SwapChain.h"

namespace ZE::GFX
{
	// Main interactions with GPU rendering objects
	class Graphics final
	{
		Device device;
		SwapChain swapChain;
		CommandList mainList;

	public:
		Graphics() = default;
		ZE_CLASS_DELETE(Graphics);
		~Graphics() { device.WaitMain(device.SetMainFence()); }

		constexpr Device& GetDevice() noexcept { return device; }
		constexpr CommandList& GetMainList() noexcept { return mainList; }
		constexpr SwapChain& GetSwapChain() noexcept { return swapChain; }
		constexpr void Present() { device.SetMainFence(); swapChain.Present(device); }

		void Init(const Window::MainWindow& window, U32 descriptorCount, U32 scratchDescriptorCount);
	};
}