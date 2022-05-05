#include "GFX/Graphics.h"

namespace ZE::GFX
{
	void Graphics::Init(const Window::MainWindow& window, U32 descriptorCount, U32 scratchDescriptorCount, bool backbufferSRV)
	{
		device.Init(descriptorCount, scratchDescriptorCount);
		mainList.Exec([&](CommandList& x) { x.InitMain(device); });
		computeList.Exec([&](CommandList& x) { x.Init(device, CommandType::Compute); });
		fenceChain.Exec([](U64& x) { x = 0; });
		swapChain.Init(window, device, backbufferSRV);
	}
}