#include "GFX/Graphics.h"

namespace ZE::GFX
{
	void Graphics::Init(const Window::MainWindow& window)
	{
		device.Init();
		mainList.Init(device);
		swapChain.Init(window, device);
	}
}