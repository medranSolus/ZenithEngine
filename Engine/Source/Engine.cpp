#include "Engine.h"

namespace ZE
{
	Engine::Engine(const char* windowName, GfxApiType gfxApi, U32 width, U32 height)
	{
		Settings::Init(gfxApi);
		window.Init(windowName, width, height);
		graphics.Init(window);
		gui.Init(graphics.GetDevice());
		renderer.Init(graphics.GetDevice(), graphics.GetSwapChain(), width, height);
		// Transform buffers: https://www.gamedev.net/forums/topic/708811-d3d12-best-approach-to-manage-constant-buffer-for-the-frame/5434325/
		// Mipmaps generation and alpha test: https://asawicki.info/articles/alpha_test.php5
	}

	void Engine::BeginFrame() const
	{
		if (IsGuiActive())
			gui.StartFrame(window);
	}

	void Engine::EndFrame()
	{
		if (IsGuiActive())
			gui.EndFrame();
		//graphics.Present();
	}
}