#include "Engine.h"

namespace ZE
{
	Engine::Engine(const char* windowName, GfxApiType gfxApi, U32 width, U32 height)
	{
		Settings::Init(gfxApi);
		window.Init(windowName, width, height);
		graphics.Init(window);
		gui.Init(graphics.GetDevice());
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