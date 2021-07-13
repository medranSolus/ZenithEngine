#include "GFX/Graphics.h"

namespace ZE::GFX
{
	void Graphics::Init(const Window::MainWindow& window)
	{
		device.Init();
		mainCtx.Init(device, true);
		swapChain.Init(window, device);
	}
}