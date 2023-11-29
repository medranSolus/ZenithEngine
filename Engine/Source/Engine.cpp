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
		// - (texture prepare) remap alpha values to following formula: max(alpha, alpha * 1/3 + shader_clamp_value * 2/3). Shader clamping value to be changed to 0.5 for tests
		// - (texture prepare) apply Dilation or Solidify filter to change object edges in texture
		// - different types of filters during mip map generation, ex. https://en.wikipedia.org/wiki/Kaiser_window
		// - check out https://research.nvidia.com/publication/2017-02_hashed-alpha-testing, http://www.ludicon.com/castano/blog/articles/computing-alpha-mipmaps/
		// Check for optimization UB code: https://github.com/xiw/stack/
		prevTime = Perf::Get().GetNow();
		assets.GetDisk().StartUploadGPU(true);

		/*
		Frontend of resource managment done in AssetsStreamer (all resource pack functions to be filled later, now let's focus on data that is being uploaded from assimp)
		  - finish saving of resource packs (retrieval of GPU data + saving to disk + compression)
		  - finish loading of other types of data: Materials, Buffers, Textures
		  - display window for managing resource packs and inspecting details about them + resources inside
		  - support for input data that is not static (will be removed before finishing upload, ex. provide normal pointer and shared_ptr as option to the input data)

		Backend of resource managment is DiskManager, request is created for upload to GPU, after succesful upload resource location is changed to GPU
		  - what if resource if freed before upload finishes? Have to postpone (delete queue till resource is not access anyway)
		  - add support for Textures (what about their initial layout between other APIs, does assets need to be created per API in this topic??)
		  - also textures after uploading to GPU need to have MIP map generation performed (maybe use SPD with technique provided above in links on async compute), only then they can be marked as on GPU
			(but in theory we could use them with worse performance and quality till mip levels are computed, so maybe better to create second resource
			that will be used during computations of MIPs while first one will only have single MIP level and after that switch will happen in the background and single level texture will be deleted)
		  - textures before copy have to be in GetCopyableFootprints1() layout which may include some padding for rows BUT to fix that we can use bufferRowLength in Vulkan to have one single layout for every API! row pitch have to be aligned to D3D12_TEXTURE_DATA_PITCH_ALIGNMENT
		  - separate queue for textures when handling their post upload setup steps

		Processing of resources have to be updated to account for their current location. It's impossible to use resource that is still being uploaded. For now sufficient enough should be to call wait
		till all resources are uploaded onto GPU and when new RenderGraph system will be enrolled then proper handling will be introduced (don't display model when data is missing).

		Need to create some form of connection between entities and meshes based on names that will be resolved after loading to memory. Single entity will need to have names of Material and Mesh saved along
		for correct identification later (can be removed after resource is loaded into memory and restored again after removing resource). For that custom scene format will be needed too for saving things like position, etc. (SceneManager).
		Also move object creation functions to SceneManager to not bloat demo app and for later reuse.

		Proper error handling with pop-ups when anything from above fails, try to remove as many possible sources of exceptions as possible. Prioritize especially in DiskManager continuity of execution with reporting error than shutdown.
		If resource failed to upload or something then mark it as on disk/memory and log it accordingly + maybe pop-up.
		*/
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
		[[maybe_unused]] bool status = assets.GetDisk().WaitForUploadGPU();
		ZE_ASSERT(status, "Error uploading engine GPU data!");
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