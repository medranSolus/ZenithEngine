#include "Engine.h"
#include "GFX/Pipeline/RenderPass/UpscaleXeSS.h"

namespace ZE
{
	void Engine::Init(const EngineParams& params)
	{
		SetGui(true);
		flags[Flags::Initialized] = true;
		ZE_PERF_CONFIGURE(SetSingleLineLogEntry, params.SingleLinePerfEntry);

		window.Init(params.WindowName ? params.WindowName : Settings::GetAppName(), params.Width, params.Height);
		Settings::DisplaySize = { window.GetWidth(), window.GetHeight() };

		graphics.Init(window, params.GraphicsDescriptorPoolSize, GFX::Pipeline::IsBackbufferSRVInRenderGraph<GFX::Pipeline::RendererPBR>::VALUE);
		Settings::RenderSize = GFX::CalculateRenderSize(graphics.GetDevice(), Settings::DisplaySize, Settings::GetUpscaler());

		gui.Init(graphics, GFX::Pipeline::IsBackbufferSRVInRenderGraph<GFX::Pipeline::RendererPBR>::VALUE);
		assets.Init(graphics.GetDevice());
		renderer.Init(graphics.GetDevice(), graphics.GetMainList(), assets, params.Renderer);
		// Transform buffers: https://www.gamedev.net/forums/topic/708811-d3d12-best-approach-to-manage-constant-buffer-for-the-frame/5434325/
		// Mipmaps generation and alpha test: https://asawicki.info/articles/alpha_test.php5
		// Reverse depth: https://www.gamedev.net/forums/topic/693404-reverse-depth-buffer/
		// For typical usage, NearZ is less than FarZ. However, if you flip these values so FarZ is less than NearZ, the result is an inverted z buffer (also known as a "reverse z buffer") which can provide increased floating-point precision.
		// 24bit depth bad: https://www.gamedev.net/forums/topic/691579-24bit-depthbuffer-is-a-sub-optimal-format/
		// Check for optimization UB code: https://github.com/xiw/stack/
		prevTime = Perf::Get().GetNow();
	}

	Engine::~Engine()
	{
		if (flags[Flags::Initialized])
		{
			// Wait till all GPU operations are done
			graphics.GetDevice().WaitMain(graphics.GetDevice().SetMainFence());

			// Free all remaining gpu data
			for (auto& buffer : Settings::Data.view<Data::DirectionalLightBuffer>())
				Settings::Data.get<Data::DirectionalLightBuffer>(buffer).Buffer.Free(graphics.GetDevice());
			for (auto& buffer : Settings::Data.view<Data::SpotLightBuffer>())
				Settings::Data.get<Data::SpotLightBuffer>(buffer).Buffer.Free(graphics.GetDevice());
			for (auto& buffer : Settings::Data.view<Data::PointLightBuffer>())
				Settings::Data.get<Data::PointLightBuffer>(buffer).Buffer.Free(graphics.GetDevice());

			for (auto& buffer : Settings::Data.view<Data::MaterialBuffersPBR>())
				Settings::Data.get<Data::MaterialBuffersPBR>(buffer).Free(graphics.GetDevice());
			for (auto& buffer : Settings::Data.view<GFX::Resource::Mesh>())
				Settings::Data.get<GFX::Resource::Mesh>(buffer).Free(graphics.GetDevice());
			renderer.Free(graphics.GetDevice());
			assets.Free(graphics.GetDevice());
			gui.Destroy(graphics.GetDevice());
		}
	}

	void Engine::Start(EID camera) noexcept
	{
		prevTime = Perf::Get().GetNow();
		renderer.SetInverseViewProjection(camera);
	}

	double Engine::BeginFrame(double deltaTime, U64 maxUpdateSteps)
	{
		ZE_PERF_START("Frame");

		double currentTime = Perf::Get().GetNow();
		double frameTime = currentTime - prevTime;
		prevTime = currentTime;
		Settings::FrameTime = frameTime;

		double maxFrameTime = deltaTime * Utils::SafeCast<double>(maxUpdateSteps);
		if (frameTime > maxFrameTime)
			frameTime = maxFrameTime;

		if (IsGuiActive())
		{
			gui.StartFrame(window);
			assets.ShowWindow(graphics.GetDevice());
		}
		return frameTime;
	}

	void Engine::EndFrame()
	{
		ZE_PERF_START("Execute render graph");
		renderer.Execute(graphics);
		ZE_PERF_STOP();

		if (IsGuiActive())
			gui.EndFrame(graphics);
		else
			graphics.GetSwapChain().PrepareBackbuffer(graphics.GetDevice(), graphics.GetMainList());

		ZE_PERF_START("Swapchain present");
		graphics.Present();
		ZE_PERF_STOP();
		// Frame marker
		ZE_PERF_STOP();
	}
}