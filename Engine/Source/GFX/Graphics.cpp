#include "GFX/Graphics.h"
#include "GFX/FfxBackendInterface.h"

namespace ZE::GFX
{
	Graphics::~Graphics()
	{
		ffxDestroyInterface(device);

		swapChain.Free(device);
		mainList.Exec([&](CommandList& cl) { cl.Free(device); });
		dynamicBuffers.Exec([&](Resource::DynamicCBuffer& buff) { buff.Free(device); });
	}

	void Graphics::Init(const Window::MainWindow& window, U32 descriptorCount, bool backbufferSRV)
	{
		device.Init(window, descriptorCount);
		mainList.Exec([&](CommandList& x) { x.InitMain(device); });
		fenceChain.Exec([](U64& x) { x = 0; });
		swapChain.Init(window, device, backbufferSRV);
		dynamicBuffers.Exec([&](Resource::DynamicCBuffer& x) { x.Init(device); });

		ffxGetInterface(device, dynamicBuffers);
	}
}