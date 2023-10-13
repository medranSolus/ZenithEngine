#include "GFX/Graphics.h"
#include "GFX/FfxBackendInterface.h"

namespace ZE::GFX
{
	void Graphics::Init(const Window::MainWindow& window, U32 descriptorCount, bool backbufferSRV)
	{
		device.Init(window, descriptorCount);
		ffxGetInterface(device);
		mainList.Exec([&](CommandList& x) { x.InitMain(device); });
		fenceChain.Exec([](U64& x) { x = 0; });
		swapChain.Init(window, device, backbufferSRV);
	}
}