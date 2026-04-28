#include "GFX/Graphics.h"

namespace ZE::GFX
{
	Expected<Graphics> Graphics::Create(const Window::MainWindow& window, U32 descriptorCount, bool backbufferSRV) noexcept
	{
		Graphics gfx;
		ZE_EXPECT_RET_FAILED(gfx.device, Device::Create(window, descriptorCount));
		ZE_EXPECT_RET_FAILED(gfx.swapChain, SwapChain::Create(window, gfx.device, backbufferSRV));
		ZE_CODE_RET_FAILED_EXPECT(gfx.mainList.ExecStatus([&](CommandList& x) -> Status { ZE_EXPECT_RET_FAILED_CODE(x, CommandList::CreateMain(gfx.device)); return {}; }));
		gfx.fenceChain.Exec([](U64& x) { x = 0; });
		return gfx;
	}

	Status Graphics::WaitForFrame() noexcept
	{
		swapChain.StartFrame(device);
		ZE_CODE_RET_FAILED(device.WaitMain(fenceChain.Get()));
		ZE_CODE_RET_FAILED(mainList.Get().Reset(device));
		return {};
	}

	Status Graphics::Present() noexcept
	{
		ZE_PERF_GUARD("Swapchain present");

		ZE_EXPECT_RET_FAILED_CODE(fenceChain.Get(), device.SetMainFence());
		ZE_CODE_RET_FAILED(swapChain.Present(device));
		device.EndFrame();
		Settings::AdvanceFrame();
		return {};
	}
}