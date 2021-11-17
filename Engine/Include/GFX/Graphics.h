#pragma once
#include "CommandList.h"
#include "Device.h"
#include "SwapChain.h"

namespace ZE::GFX
{
	// Main interactions with GPU rendering objects
	class Graphics final
	{
		SwapChain swapChain;
		CommandList mainList;
		Device device;

	public:
		Graphics() = default;
		Graphics(Graphics&&) = delete;
		Graphics(const Graphics&) = delete;
		Graphics& operator=(Graphics&&) = delete;
		Graphics& operator=(const Graphics&) = delete;
		~Graphics() = default;

		constexpr Device& GetDevice() noexcept { return device; }
		constexpr CommandList& GetMainList() noexcept { return mainList; }
		constexpr SwapChain& GetSwapChain() noexcept { return swapChain; }
		constexpr void Present();

		void Init(const Window::MainWindow& window, U32 descriptorCount, U32 scratchDescriptorCount);
	};

#pragma region Functions
	constexpr void Graphics::Present()
	{
		device.WaitMain(device.SetMainFence());
		swapChain.Present(device);
		mainList.Reset(device);
	}
#pragma endregion
}