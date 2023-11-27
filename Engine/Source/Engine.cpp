#include "Engine.h"
ZE_WARNING_PUSH
#include "nvsdk_ngx_helpers.h"
ZE_WARNING_POP

namespace ZE
{
	void NVSDK_CONV AllocNGXResourceD3D12(D3D12_RESOURCE_DESC* InDesc, int InState, CD3DX12_HEAP_PROPERTIES* InHeap, ID3D12Resource** OutResource) noexcept
	{
	}

	void NVSDK_CONV AllocNGXBufferD3D11(D3D11_BUFFER_DESC* InDesc, ID3D11Buffer** OutResource) noexcept
	{
	}

	void NVSDK_CONV AllocNGXTexture2DD3D11(D3D11_TEXTURE2D_DESC* InDesc, ID3D11Texture2D** OutResource) noexcept
	{
	}

	void NVSDK_CONV ReleaseNGXResourceD3D(IUnknown* InResource) noexcept
	{
	}

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
		// Check for optimization UB code: https://github.com/xiw/stack/
		prevTime = Perf::Get().GetNow();
		// https://www.a23d.co/blog/different-maps-in-pbr-textures
		// https://help.poliigon.com/en/articles/1712652-texture-maps-explained
		// https://stackoverflow.com/questions/48494389/how-does-this-code-sample-from-a-spherical-map

		// Scenes:
		// Intel Sponza - https://www.intel.com/content/www/us/en/developer/topic-technology/graphics-research/samples.html
		// PBRT - https://pbrt.org/scenes-v3
		// Benedikt - https://benedikt-bitterli.me/resources/
		// NVidia - https://developer.nvidia.com/orca
		// OBJ Sponza - https://www.alexandre-pestana.com/pbr-textures-sponza/

		/*
		Frontend of resource managment done in AssetsStreamer
		  - finish saving of resource packs (retrieval of GPU data + saving to disk + compression)
		  - display window for managing resource packs and inspecting details about them + resources inside

		Backend of resource managment is DiskManager, request is created for upload to GPU, after succesful upload resource location is changed to GPU
		  - what if resource if freed before upload finishes? Have to postpone (delete queue till resource is not access anyway)
		  - separate queue for textures when handling their post upload setup steps

		Processing of resources have to be updated to account for their current location. It's impossible to use resource that is still being uploaded. For now sufficient enough should be to call wait
		till all resources are uploaded onto GPU and when new RenderGraph system will be enrolled then proper handling will be introduced (don't display model when data is missing).

		Need to create some form of connection between entities and meshes based on names that will be resolved after loading to memory. Single entity will need to have names of Material and Mesh saved along
		for correct identification later (can be removed after resource is loaded into memory and restored again after removing resource). For that custom scene format will be needed too for saving things like position, etc. (SceneManager).
		Also move object creation functions to SceneManager to not bloat demo app and for later reuse.

		Proper error handling with pop-ups when anything from above fails, try to remove as many possible sources of exceptions as possible. Prioritize especially in DiskManager continuity of execution with reporting error than shutdown.
		If resource failed to upload or something then mark it as on disk/memory and log it accordingly + maybe pop-up.

		MIP generation:
		  - textures after uploading to GPU can have MIP map generation performed (maybe use SPD with technique provided above in links on async compute) as additional step to be triggered in editor
			(but in theory we could use them with worse performance and quality till mip levels are computed, so maybe better to create second resource
			that will be used during computations of MIPs while first one will only have single MIP level and after that switch will happen in the background and single level texture will be deleted)
		  - alpha test: https://asawicki.info/articles/alpha_test.php5
		  - (texture prepare) remap alpha values to following formula: max(alpha, alpha * 1/3 + shader_clamp_value * 2/3). Shader clamping value to be changed to 0.5 for tests
		  - (texture prepare) apply Dilation or Solidify filter to change object edges in texture
		  - different types of filters during mip map generation, ex. https://en.wikipedia.org/wiki/Kaiser_window
		  - check out https://research.nvidia.com/publication/2017-02_hashed-alpha-testing, http://www.ludicon.com/castano/blog/articles/computing-alpha-mipmaps/
		  - bigger concern, maybe create offline module to handle creation of mipmaps since they can be computed manually by artist?
		*/

		NVSDK_NGX_FeatureDiscoveryInfo info = {};
		info.SDKVersion = NVSDK_NGX_Version_API;
		info.ApplicationDataPath = Logger::LOG_DIR_W;
		info.FeatureInfo = &featureInfo;
#if ZE_NGX_ID
		info.Identifier.IdentifierType = NVSDK_NGX_Application_Identifier_Type_Application_Id;
		info.Identifier.v.ApplicationId = ZE_NGX_ID;
#else
		info.Identifier.IdentifierType = NVSDK_NGX_Application_Identifier_Type_Project_Id;
		info.Identifier.v.ProjectDesc.ProjectId = Settings::ENGINE_UUID;
		info.Identifier.v.ProjectDesc.EngineType = NVSDK_NGX_ENGINE_TYPE_CUSTOM;
		info.Identifier.v.ProjectDesc.EngineVersion = Settings::ENGINE_VERSION_STR;
#endif

		NVSDK_NGX_Parameter* dlssParams = nullptr;

		// After successful check for given feature, put job on different thread to update it for next run (if possible)
		{
			NVSDK_NGX_Result resUpdate = NVSDK_NGX_UpdateFeature(&info.Identifier, info.FeatureID);
			int a = 0;
		}
		// Initialize DLSS
		NVSDK_NGX_Handle* featureHandle = nullptr;
		// Execute DLSS
		if (false)
		{
			dlssParams->Set(NVSDK_NGX_Parameter_Color, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_Output, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_Depth, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_MotionVectors, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_TransparencyMask, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_ExposureTexture, (ID3D12Resource*)nullptr); // Using auto exposure, but give as option
			// Use on animated textures, small particles, thin objects, huge or missing motion vectors (only after encountering problems after integration)
			dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (ID3D12Resource*)nullptr);

			dlssParams->Set(NVSDK_NGX_Parameter_Jitter_Offset_X, Data::GetUnitPixelJitterX(renderer.GetProjectionData().JitterX, Settings::RenderSize.X));
			dlssParams->Set(NVSDK_NGX_Parameter_Jitter_Offset_Y, Data::GetUnitPixelJitterY(renderer.GetProjectionData().JitterY, Settings::RenderSize.Y));
			dlssParams->Set(NVSDK_NGX_Parameter_Reset, 0U); // TODO: Check conditions for that
			dlssParams->Set(NVSDK_NGX_Parameter_MV_Scale_X, -Utils::SafeCast<float>(Settings::RenderSize.X));
			dlssParams->Set(NVSDK_NGX_Parameter_MV_Scale_Y, -Utils::SafeCast<float>(Settings::RenderSize.Y));
			dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Width, Settings::RenderSize.X);
			dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Height, Settings::RenderSize.Y);
			dlssParams->Set(NVSDK_NGX_Parameter_FrameTimeDeltaInMsec, Settings::FrameTime);

			// Optional future parameters if engine can expose them
			dlssParams->Set(NVSDK_NGX_Parameter_GBuffer_Albedo, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_GBuffer_Roughness, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_GBuffer_Metallic, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_GBuffer_Specular, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_GBuffer_Subsurface, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_GBuffer_Normals, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_GBuffer_ShadingModelId, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_GBuffer_MaterialId, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_GBuffer_Atrrib_8, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_GBuffer_Atrrib_9, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_GBuffer_Atrrib_10, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_GBuffer_Atrrib_11, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_GBuffer_Atrrib_12, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_GBuffer_Atrrib_13, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_GBuffer_Atrrib_14, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_GBuffer_Atrrib_15, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_TonemapperType, NVSDK_NGX_TONEMAPPER_STRING);
			dlssParams->Set(NVSDK_NGX_Parameter_MotionVectors3D, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_IsParticleMask, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_AnimatedTextureMask, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_DepthHighRes, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_Position_ViewSpace, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_RayTracingHitDistance, (ID3D12Resource*)nullptr);
			dlssParams->Set(NVSDK_NGX_Parameter_MotionVectorsReflection, (ID3D12Resource*)nullptr);

			// Render
			graphics.GetMainList().Open(graphics.GetDevice());
			NVSDK_NGX_Result resExec = NVSDK_NGX_D3D12_EvaluateFeature(graphics.GetMainList().Get().dx12.GetList(), featureHandle, dlssParams, nullptr);
			graphics.GetMainList().Close(graphics.GetDevice());
			graphics.GetDevice().ExecuteMain(graphics.GetMainList());
		}
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
			Settings::Data.clear<Data::DirectionalLightBuffer>();
			for (auto& buffer : Settings::Data.view<Data::SpotLightBuffer>())
				Settings::Data.get<Data::SpotLightBuffer>(buffer).Buffer.Free(graphics.GetDevice());
			Settings::Data.clear<Data::SpotLightBuffer>();
			for (auto& buffer : Settings::Data.view<Data::PointLightBuffer>())
				Settings::Data.get<Data::PointLightBuffer>(buffer).Buffer.Free(graphics.GetDevice());
			Settings::Data.clear<Data::PointLightBuffer>();

			for (auto& buffer : Settings::Data.view<Data::MaterialBuffersPBR>())
				Settings::Data.get<Data::MaterialBuffersPBR>(buffer).Free(graphics.GetDevice());
			Settings::Data.clear<Data::MaterialBuffersPBR>();
			for (auto& buffer : Settings::Data.view<GFX::Resource::Mesh>())
				Settings::Data.get<GFX::Resource::Mesh>(buffer).Free(graphics.GetDevice());
			Settings::Data.clear<GFX::Resource::Mesh>();

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
		assets.GetDisk().StartUploadGPU(true);
		return frameTime;
	}

	void Engine::EndFrame()
	{
		graphics.WaitForFrame();
		GFX::Device& dev = graphics.GetDevice();
		GFX::CommandList& cl = graphics.GetMainList();

		ZE_PERF_START("Update upload data status");
		const bool gpuWorkPending = assets.GetDisk().IsGPUWorkPending();
		if (gpuWorkPending)
			cl.Open(dev);

		[[maybe_unused]] bool status = assets.GetDisk().WaitForUploadGPU(dev, cl);
		ZE_ASSERT(status, "Error uploading engine GPU data!");

		if (gpuWorkPending)
		{
			cl.Close(dev);
			dev.ExecuteMain(cl);
		}
		ZE_PERF_STOP();

		ZE_PERF_START("Execute render graph");
		renderer.Execute(graphics);
		ZE_PERF_STOP();

		if (IsGuiActive())
			gui.EndFrame(graphics);
		else
			graphics.GetSwapChain().PrepareBackbuffer(dev, cl);

		ZE_PERF_START("Swapchain present");
		graphics.Present();
		ZE_PERF_STOP();
		// Frame marker
		ZE_PERF_STOP();
	}
}