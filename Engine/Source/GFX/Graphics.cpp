#include "GFX/Graphics.h"

namespace ZE::GFX
{
	void Graphics::Init(const Window::MainWindow& window, U32 descriptorCount, U32 scratchDescriptorCount)
	{
		device.Init(descriptorCount, scratchDescriptorCount);
		mainList.Exec([&](CommandList& x) { x.Init(device, CommandType::All); });
		fenceChain.Exec([](U64& x) { x = 0; });
		swapChain.Init(window, device);
	}
}