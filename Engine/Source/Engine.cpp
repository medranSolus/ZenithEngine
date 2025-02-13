#include "Engine.h"
#include "Data/Light.h"
#include "Data/Transform.h"

namespace ZE
{
	bool Engine::UploadSync()
	{
		GFX::Device& dev = graphics.GetDevice();
		GFX::CommandList& mainList = graphics.GetMainList();
		DiskStatusHandle diskStatus = assets.GetDisk().SetGPUUploadWaitPoint();
		assets.GetDisk().StartUploadGPU();

		ZE_PERF_START("Update upload data status");
		const bool gpuWorkPending = assets.GetDisk().IsGPUWorkPending(diskStatus);
		if (gpuWorkPending)
			mainList.Open(dev);

		[[maybe_unused]] bool status = assets.GetDisk().WaitForUploadGPU(dev, mainList, diskStatus);
		ZE_ASSERT(status, "Error uploading engine GPU data!");
		flags[ExecuteUploadSync] = false;

		if (gpuWorkPending)
			mainList.Close(dev);
		ZE_PERF_STOP();
		return gpuWorkPending;
	}

	bool Engine::Init(const EngineParams& params)
	{
		flags[Flags::Initialized] = true;
		ZE_PERF_CONFIGURE(SetSingleLineLogEntry, params.SingleLinePerfEntry);

		window.Init(params.WindowName ? params.WindowName : Settings::GetAppName(), params.Width, params.Height);
		Settings::DisplaySize = { window.GetWidth(), window.GetHeight() };

		graphics.Init(window, params.GraphicsDescriptorPoolSize, true); // GFX::Pipeline::IsBackbufferSRVInRenderGraph<GFX::Pipeline::RendererPBR>::VALUE);
		GFX::Device& dev = graphics.GetDevice();
		Settings::RenderSize = GFX::CalculateRenderSize(dev, Settings::DisplaySize, Settings::Upscaler, UINT32_MAX);

		assets.Init(dev);

		GFX::Pipeline::BuildResult buildRes = GFX::Pipeline::BuildResult::Success;
		if (params.CustomRendererDesc)
		{
			buildRes = graphBuilder.LoadConfig(dev, *params.CustomRendererDesc);
			if (buildRes == GFX::Pipeline::BuildResult::Success)
			{
				buildRes = graphBuilder.ComputeGraph(dev);
				if (buildRes == GFX::Pipeline::BuildResult::Success)
					graphBuilder.FinalizeGraph(dev, assets, renderGraph);
			}
		}
		else
		{
			buildRes = graphBuilder.LoadConfig(dev, GFX::Pipeline::CoreRenderer::GetDesc(params.CoreRendererParams));
			if (buildRes == GFX::Pipeline::BuildResult::Success)
			{
				buildRes = graphBuilder.ComputeGraph(dev);
				if (buildRes == GFX::Pipeline::BuildResult::Success)
					graphBuilder.FinalizeGraph(dev, assets, renderGraph);
			}
		}
		if (buildRes != GFX::Pipeline::BuildResult::Success)
		{
			Logger::Error(std::string("Error processing render gragh, reason: ") + GFX::Pipeline::DecodeBuildResult(buildRes));
			return false;
		}

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
		return true;
	}

	Engine::~Engine()
	{
		if (flags[Flags::Initialized])
		{
			// Wait till all GPU operations are done
			GFX::Device& dev = graphics.GetDevice();
			dev.FlushGPU();

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

			renderGraph.Free(dev);
			graphBuilder.ClearConfig(dev);
			assets.Free(dev);
		}
	}

	void Engine::Start(EID camera) noexcept
	{
		renderGraph.SetCamera(camera);
		UploadSync();
		prevTime = Perf::Get().GetNow();
	}

	void Engine::ShowRenderGraphDebugUI() noexcept
	{
		if (ImGui::BeginChild("##render_graph_settings"))
		{
			if (ImGui::CollapsingHeader("Engine info"))
			{
				ImGui::Text(Settings::ENGINE_DISPLAY_NAME);
				ImGui::Text("Version:"); ImGui::SameLine(); ImGui::Text(Settings::ENGINE_VERSION_STR);
				ImGui::Text("UUID:"); ImGui::SameLine(); ImGui::Text(Settings::ENGINE_UUID);

				ImGui::Text("Current RHI:"); ImGui::SameLine();
				switch (Settings::GetGfxApi())
				{
				case GfxApiType::DX11:
				{
					ImGui::Text("DirectX 11");
					break;
				}
				case GfxApiType::DX12:
				{
					ImGui::Text("DirectX 12");
					break;
				}
				case GfxApiType::OpenGL:
				{
					ImGui::Text("OpenGL");
					break;
				}
				case GfxApiType::Vulkan:
				{
					ImGui::Text("Vulkan");
					break;
				}
				}

				ImGui::NewLine();
			}

			if (ImGui::CollapsingHeader("Effects"))
			{
				constexpr std::array<const char*, 3> AO_LEVELS = { "No AO", "XeGTAO", "CACAO" };
				if (ImGui::BeginCombo("Ambient Occlusion", AO_LEVELS.at(static_cast<U8>(Settings::AmbientOcclusionType))))
				{
					for (GFX::AOType i = GFX::AOType::None; const char* level : AO_LEVELS)
					{
						const bool selected = i == Settings::AmbientOcclusionType;
						if (ImGui::Selectable(level, selected))
							Settings::AmbientOcclusionType = i;
						if (selected)
							ImGui::SetItemDefaultFocus();
						i = static_cast<GFX::AOType>(static_cast<U8>(i) + 1);
					}
					ImGui::EndCombo();
				}

				constexpr std::array<const char*, 7> UPSCALE_LEVELS = { "None", "FSR 1", "FSR 2", "FSR 3", "XeSS", "NIS", "DLSS" };
				if (ImGui::BeginCombo("Upscaling", UPSCALE_LEVELS.at(static_cast<U8>(Settings::Upscaler))))
				{
					for (GFX::UpscalerType i = GFX::UpscalerType::None; const char* level : UPSCALE_LEVELS)
					{
						if (GFX::IsUpscalerSupported(graphics.GetDevice(), i))
						{
							const bool selected = i == Settings::Upscaler;
							if (ImGui::Selectable(level, selected))
								Settings::Upscaler = i;
							if (selected)
								ImGui::SetItemDefaultFocus();
							i = static_cast<GFX::UpscalerType>(static_cast<U8>(i) + 1);
						}
					}
					ImGui::EndCombo();
				}
				ImGui::NewLine();
			}
			if (graphBuilder.ShowCurrentPassesDebugUI(graphics.GetDevice(), assets, renderGraph))
				flags[ExecuteUploadSync] = true;
		}
		ImGui::EndChild();
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

		if (Settings::IsEnabledImGui())
		{
			imgui.StartFrame(window);
			assets.ShowWindow(graphics.GetDevice());
		}
		assets.GetDisk().StartUploadGPU();
		return frameTime;
	}

	void Engine::EndFrame()
	{
		// TODO: add module for checking if all settings parameters are correct and decide on subsystems updates (like changing gfx API, swapchain recreation)

		GFX::Device& dev = graphics.GetDevice();
		GFX::CommandList& mainList = graphics.GetMainList();

		if (Settings::IsEnabledImGui())
			imgui.EndFrame();
		graphics.WaitForFrame();

		// Update of render graph and it's data
		GFX::Pipeline::BuildResult result = graphBuilder.UpdatePassConfiguration(dev, assets, renderGraph);
		if (!ZE_PIPELINE_BUILD_SUCCESS(result))
			throw ZE_RGC_EXCEPT("Error performing update on a render graph: " + std::string(GFX::Pipeline::DecodeBuildResult(result)));
		renderGraph.UpdateFrameData(dev);

		if (result == GFX::Pipeline::BuildResult::WaitUpload || flags[ExecuteUploadSync])
		{
			// If any async passes has been processed then sync for initialization of resources
			if (UploadSync() && renderGraph.IsAsyncPresent())
			{
				dev.ExecuteMain(mainList);
				dev.WaitComputeFromMain(dev.SetMainFence());
			}
		}

		// Add or remove missing transform components
		if (Settings::ComputeMotionVectors())
		{
			if (!Settings::Data.all_of<Data::TransformPrevious>(renderGraph.GetCurrentCamera()))
			{
				for (EID id : Settings::Data.view<Data::TransformGlobal>())
					Settings::Data.emplace_or_replace<Data::TransformPrevious>(id, Settings::Data.get<Data::TransformGlobal>(id));
			}
		}
		else if (Settings::Data.all_of<Data::TransformPrevious>(renderGraph.GetCurrentCamera()))
			Settings::Data.clear<Data::TransformPrevious>();

		renderGraph.Execute(graphics);

		// Move all current transforms to previous state
		if (Settings::ComputeMotionVectors())
		{
			for (EID id : Settings::Data.view<Data::TransformGlobal>())
				static_cast<Data::TransformGlobal&>(Settings::Data.get<Data::TransformPrevious>(id)) = Settings::Data.get<Data::TransformGlobal>(id);
		}
		graphics.Present();

		// Frame marker
		ZE_PERF_STOP();
	}
}