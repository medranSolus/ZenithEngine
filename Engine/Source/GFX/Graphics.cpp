#include "GFX/Graphics.h"

namespace ZE::GFX
{
	Graphics::~Graphics()
	{
		delete device;
		delete mainCtx;
		delete swapChain;
	}

	void Graphics::Init(const Window::MainWindow& window)
	{
		device = Settings::GetGfxApi().MakeDevice();
		mainCtx = Settings::GetGfxApi().MakeMainContext(*device);
		swapChain = Settings::GetGfxApi().MakeSwapChain(window, *device);
	}

	void Graphics::Present() const
	{
		swapChain->Present(*device);
	}
}