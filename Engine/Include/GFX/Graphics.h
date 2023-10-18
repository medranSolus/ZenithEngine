#pragma once
#include "Resource/DynamicCBuffer.h"
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
		// TODO: Move chainPool inside CL
		ChainPool<CommandList> mainList;
		ChainPool<U64> fenceChain;
		ChainPool<Resource::DynamicCBuffer> dynamicBuffers;

	public:
		Graphics() = default;
		ZE_CLASS_DELETE(Graphics);
		~Graphics();

		constexpr Device& GetDevice() noexcept { return device; }
		constexpr CommandList& GetMainList() noexcept { return mainList.Get(); }
		constexpr SwapChain& GetSwapChain() noexcept { return swapChain; }
		constexpr Resource::DynamicCBuffer& GetDynamicBuffer() noexcept { return dynamicBuffers.Get(); }
		constexpr void WaitForFrame();
		constexpr void Present();

		void Init(const Window::MainWindow& window, U32 descriptorCount, bool backbufferSRV);
	};

#pragma region Functions
	constexpr void Graphics::WaitForFrame()
	{
		swapChain.StartFrame(device);
		device.WaitMain(fenceChain.Get());
		dynamicBuffers.Get().StartFrame(device);
		mainList.Get().Reset(device);
	}

	constexpr void Graphics::Present()
	{
		fenceChain.Get() = device.SetMainFence();
		swapChain.Present(device);
		device.EndFrame();
		Settings::AdvanceFrame();
	}
#pragma endregion
}