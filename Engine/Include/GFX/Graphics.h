#pragma once
#include "ChainPool.h"
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
		ChainPool<CommandList> mainList;
		ChainPool<CommandList> computeList;
		ChainPool<U64> fenceChain;

	public:
		Graphics() = default;
		ZE_CLASS_DELETE(Graphics);
		~Graphics() = default;

		constexpr Device& GetDevice() noexcept { return device; }
		constexpr CommandList& GetMainList() noexcept { return mainList.Get(); }
		constexpr CommandList& GetComputeList() noexcept { return computeList.Get(); }
		constexpr SwapChain& GetSwapChain() noexcept { return swapChain; }
		constexpr void WaitForFrame() noexcept { return device.WaitMain(fenceChain.Get()); }
		constexpr void Present();

		void Init(const Window::MainWindow& window, U32 descriptorCount, U32 scratchDescriptorCount, bool backbufferSRV);
	};

#pragma region Functions
	constexpr void Graphics::Present()
	{
		fenceChain.Get() = device.SetMainFence();
		swapChain.Present(device);
		Settings::AdvanceFrame();
	}
#pragma endregion
}