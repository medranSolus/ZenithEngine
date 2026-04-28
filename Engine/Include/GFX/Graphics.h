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
		ChainPool<U64> fenceChain;

	public:
		Graphics() = default;
		ZE_CLASS_MOVE(Graphics);
		~Graphics() = default;

		static Expected<Graphics> Create(const Window::MainWindow& window, U32 descriptorCount, bool backbufferSRV) noexcept;

		constexpr Device& GetDevice() noexcept { return device; }
		constexpr CommandList& GetMainList() noexcept { return mainList.Get(); }
		constexpr SwapChain& GetSwapChain() noexcept { return swapChain; }

		Status WaitForFrame() noexcept;
		Status Present() noexcept;
	};
}