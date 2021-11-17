#include "Engine.h"

namespace ZE
{
	Engine::Engine(const EngineParams& params)
	{
		Settings::Init(params.GraphicsAPI, params.BackbufferCount);
		window.Init(params.WindowName, params.Width, params.Height);
		graphics.Init(window, params.GraphicsDescriptorPoolSize, params.ScratchDescriptorCount);
		gui.Init(graphics.GetDevice());
		renderer.Init(graphics.GetDevice(), graphics.GetMainList(),
			params.Width, params.Height, params.MinimizeRenderPassDistances, params.ShadowMapSize);
		// Transform buffers: https://www.gamedev.net/forums/topic/708811-d3d12-best-approach-to-manage-constant-buffer-for-the-frame/5434325/
		// Mipmaps generation and alpha test: https://asawicki.info/articles/alpha_test.php5
	}

	void Engine::BeginFrame()
	{
		if (IsGuiActive())
			gui.StartFrame(window);
		renderer.StartFrame(graphics.GetDevice(), graphics.GetSwapChain());
	}

	void Engine::EndFrame()
	{
		//renderer.Execute(graphics.GetDevice(), graphics.GetMainList());
		if (IsGuiActive())
			gui.EndFrame(graphics.GetMainList());
		//graphics.Present();
	}
}