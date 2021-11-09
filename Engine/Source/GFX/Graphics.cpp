#include "GFX/Graphics.h"

namespace ZE::GFX
{
	void Graphics::Init(const Window::MainWindow& window, U32 descriptorCount, U32 scratchDescriptorCount)
	{
		device.Init(descriptorCount, scratchDescriptorCount);
		mainList.Init(device);
		swapChain.Init(window, device);
	}
}