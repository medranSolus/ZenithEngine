#include "Engine.h"

namespace ZE
{
	Engine::Engine(const EngineParams& params) : StartupConfig(params)
	{
		window.Init(params.WindowName, params.Width, params.Height);
		graphics.Init(window, params.GraphicsDescriptorPoolSize, params.ScratchDescriptorCount);
		gui.Init(graphics.GetDevice());
		renderer.Init(graphics.GetDevice(), graphics.GetMainList(), textureLib,
			window.GetWidth(), window.GetHeight(), params.Renderer);
		// Transform buffers: https://www.gamedev.net/forums/topic/708811-d3d12-best-approach-to-manage-constant-buffer-for-the-frame/5434325/
		// Mipmaps generation and alpha test: https://asawicki.info/articles/alpha_test.php5
		// Reverse depth: https://www.gamedev.net/forums/topic/693404-reverse-depth-buffer/
		// For typical usage, NearZ is less than FarZ. However, if you flip these values so FarZ is less than NearZ, the result is an inverted z buffer (also known as a "reverse z buffer") which can provide increased floating-point precision.
		// 24bit depth bad: https://www.gamedev.net/forums/topic/691579-24bit-depthbuffer-is-a-sub-optimal-format/
		// Check for optimization UB code: https://github.com/xiw/stack/
	}

	Engine::~Engine()
	{
		// Wait till all GPU operations are done
		graphics.GetDevice().WaitMain(graphics.GetDevice().SetMainFence());

		// Free all remaining gpu data
		for (auto& buffer : GetData().view<Data::DirectionalLightBuffer>())
			GetData().get<Data::DirectionalLightBuffer>(buffer).Buffer.Free(graphics.GetDevice());
		for (auto& buffer : GetData().view<Data::SpotLightBuffer>())
			GetData().get<Data::SpotLightBuffer>(buffer).Buffer.Free(graphics.GetDevice());
		for (auto& buffer : GetData().view<Data::PointLightBuffer>())
			GetData().get<Data::PointLightBuffer>(buffer).Buffer.Free(graphics.GetDevice());

		for (auto& buffer : GetResourceData().view<Data::MaterialBuffersPBR>())
			GetResourceData().get<Data::MaterialBuffersPBR>(buffer).Free(graphics.GetDevice());
		for (auto& buffer : GetResourceData().view<Data::Geometry>())
		{
			auto& geometry = GetResourceData().get<Data::Geometry>(buffer);
			geometry.Indices.Free(graphics.GetDevice());
			geometry.Vertices.Free(graphics.GetDevice());
		}
	}

	void Engine::BeginFrame()
	{
		if (IsGuiActive())
			gui.StartFrame(window);
	}

	void Engine::EndFrame()
	{
		renderer.Execute(graphics);
		if (IsGuiActive())
			gui.EndFrame(graphics);
		else
			graphics.GetSwapChain().PrepareBackbuffer(graphics.GetDevice(), graphics.GetMainList());
		graphics.Present();
	}
}