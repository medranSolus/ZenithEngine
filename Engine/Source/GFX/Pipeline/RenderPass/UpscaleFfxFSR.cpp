#include "GFX/Pipeline/RenderPass/UpscaleFfxFSR.h"
#include "Data/Camera.h"
#include "GUI/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::UpscaleFfxFSR
{
	static UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats) { return Update(dev, buildData, *reinterpret_cast<ExecuteData*>(passData)); }

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData) { return Initialize(dev, buildData); }

	static void MessageHandler(U32 type, const wchar_t* msg) noexcept
	{
		switch (type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case FFX_API_MESSAGE_TYPE_ERROR:
			Logger::Error(Utils::ToUTF8(msg));
			break;
		case FFX_API_MESSAGE_TYPE_WARNING:
			Logger::Warning(Utils::ToUTF8(msg));
			break;
		}
	}

	PassDesc GetDesc() noexcept
	{
		PassDesc desc{ Base(CorePassType::UpscaleFfxFSR) };
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.Clean = Clean;
		desc.DebugUI = DebugUI;
		return desc;
	}

	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus)
	{
		const FfxApiFunctions* ffxFunc = dev.GetFfxFunctions();
		if (ffxFunc)
		{
			ZE_FFX_API_ENABLE();
			Settings::RenderSize = Settings::DisplaySize;
			syncStatus.SyncMain(dev);
			ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
			ZE_FFX_API_CHECK(ffxFunc->DestroyContext(&execData->Ctx, nullptr), "Error destroying FFX FSR context!");
			delete execData;
		}
	}

	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, bool firstUpdate)
	{
		const FfxApiFunctions* ffxFunc = dev.GetFfxFunctions();
		if (ffxFunc)
		{
			UInt2 renderSize = CalculateRenderSize(dev, Settings::DisplaySize, UpscalerType::FfxFsr, passData.Quality);
			if (renderSize != Settings::RenderSize || passData.DisplaySize != Settings::DisplaySize || passData.PrevSelectedVersion != passData.SelectedVersion)
			{
				ZE_FFX_API_ENABLE();
				Settings::RenderSize = renderSize;
				passData.DisplaySize = Settings::DisplaySize;

				if (firstUpdate)
				{
					// Get possible versions of the FSR
					ffxQueryDescGetVersions versionQuery = { FFX_API_QUERY_DESC_TYPE_GET_VERSIONS, nullptr };
					versionQuery.createDescType = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;
					versionQuery.device = dev.GetFfxHandle();
					versionQuery.outputCount = &passData.VersionCount;
					versionQuery.versionIds = nullptr;
					versionQuery.versionNames = nullptr;
					ZE_FFX_API_CHECK(ffxFunc->Query(nullptr, &versionQuery.header), "Error getting possible number of FFX-API FSR versions!");

					passData.VersionIds = std::make_unique<U64[]>(passData.VersionCount);
					passData.VersionNames = std::make_unique<const char* []>(passData.VersionCount + 1);
					if (ZE_FFX_API_EXCEPT_RESULT == FFX_API_RETURN_OK)
					{
						versionQuery.versionIds = passData.VersionIds.get();
						versionQuery.versionNames = passData.VersionNames.get();
						ZE_FFX_API_CHECK(ffxFunc->Query(nullptr, &versionQuery.header), "Error getting FFX-API FSR versions!");
						passData.SelectedVersion = passData.VersionCount;
					}
					passData.VersionNames[passData.VersionCount] = "Driver default";
				}
				else
				{
					buildData.SyncStatus.SyncMain(dev);
					ZE_FFX_API_CHECK(ffxFunc->DestroyContext(&passData.Ctx, nullptr), "Error destroying FFX-API FSR context!");
				}
				passData.PrevSelectedVersion = passData.SelectedVersion;

				ffxOverrideVersion versionDesc = { FFX_API_DESC_TYPE_OVERRIDE_VERSION, nullptr };
				ffxCreateContextDescUpscale ctxDesc = { FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE, nullptr };
				if (passData.SelectedVersion < passData.VersionCount)
				{
					versionDesc.versionId = passData.VersionIds[passData.SelectedVersion];
					ctxDesc.header.pNext = &versionDesc.header;
				}
				ctxDesc.flags = FFX_UPSCALE_ENABLE_DEPTH_INVERTED | FFX_UPSCALE_ENABLE_DEPTH_INFINITE
					| FFX_UPSCALE_ENABLE_AUTO_EXPOSURE | FFX_UPSCALE_ENABLE_HIGH_DYNAMIC_RANGE
					| (_ZE_DEBUG_GFX_API ? FFX_UPSCALE_ENABLE_DEBUG_CHECKING : 0);
				ctxDesc.maxRenderSize = { renderSize.X, renderSize.Y };
				ctxDesc.maxUpscaleSize = { passData.DisplaySize.X, passData.DisplaySize.Y };
				ctxDesc.fpMessage = MessageHandler;
				ZE_FFX_API_CHECK(dev.CreateFfxCtx(&passData.Ctx, ctxDesc.header), "Error creating FFX-API FSR context!");
				return UpdateStatus::FrameBufferImpact;
			}
		}
		return UpdateStatus::NoUpdate;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData)
	{
		ExecuteData* passData = new ExecuteData;
		Update(dev, buildData, *passData, true);
		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		const FfxApiFunctions* ffxFunc = dev.GetFfxFunctions();
		if (ffxFunc)
		{
			ZE_FFX_API_ENABLE();
			ZE_PERF_GUARD("Upscale FFX-API FSR");

			Resources ids = *passData.Resources.CastConst<Resources>();
			ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();
			const UInt2 inputSize = renderData.Buffers.GetDimmensions(ids.Color);
			const UInt2 outputSize = renderData.Buffers.GetDimmensions(ids.Output);

			ZE_DRAW_TAG_BEGIN(dev, cl, "Upscale FFX-API FSR", Pixel(0xB2, 0x22, 0x22));

			ffxDispatchDescUpscale desc = { FFX_API_DISPATCH_DESC_TYPE_UPSCALE, nullptr };
			desc.commandList = cl.GetFfxHandle();
			desc.color = renderData.Buffers.GetFfxResource(ids.Color, FFX_API_RESOURCE_STATE_COMPUTE_READ);
			desc.depth = renderData.Buffers.GetFfxResource(ids.Depth, FFX_API_RESOURCE_STATE_COMPUTE_READ);
			desc.motionVectors = renderData.Buffers.GetFfxResource(ids.MotionVectors, FFX_API_RESOURCE_STATE_COMPUTE_READ);
			desc.exposure.resource = nullptr;
			desc.reactive = renderData.Buffers.GetFfxResource(ids.ReactiveMask, FFX_API_RESOURCE_STATE_COMPUTE_READ);
			desc.transparencyAndComposition.resource = nullptr; // Alpha value for special surfaces (reflections, animated textures, etc.), add when needed
			desc.output = renderData.Buffers.GetFfxResource(ids.Output, FFX_API_RESOURCE_STATE_UNORDERED_ACCESS);
			desc.jitterOffset.x = Data::GetUnitPixelJitterX(renderData.DynamicData.JitterCurrent.x, Settings::RenderSize.X);
			desc.jitterOffset.y = Data::GetUnitPixelJitterY(renderData.DynamicData.JitterCurrent.y, Settings::RenderSize.Y);
			desc.motionVectorScale.x = -Utils::SafeCast<float>(inputSize.X);
			desc.motionVectorScale.y = -Utils::SafeCast<float>(inputSize.Y);
			desc.renderSize = { inputSize.X, inputSize.Y };
			desc.upscaleSize = { outputSize.X, outputSize.Y };
			desc.enableSharpening = data.SharpeningEnabled;
			desc.sharpness = data.Sharpness;
			desc.frameTimeDelta = Utils::SafeCast<float>(Settings::FrameTime);
			desc.preExposure = 1.0f;
			desc.reset = renderData.GraphData.FrameTemporalReset;
			desc.cameraNear = FLT_MAX;
			desc.cameraFar = renderData.DynamicData.NearClip;
			desc.cameraFovAngleVertical = Settings::Data.get<Data::Camera>(renderData.GraphData.CurrentCamera).Projection.FOV;
			desc.viewSpaceToMetersFactor = 1.0f;
			desc.flags = 0;
			ZE_FFX_API_CHECK(ffxFunc->Dispatch(&data.Ctx, &desc.header), "Error performing FFX-API FSR!");
			cl.RestoreExternalState(dev);

			ZE_DRAW_TAG_END(dev, cl);
		}
	}

	void DebugUI(void* data) noexcept
	{
		if (ImGui::CollapsingHeader("FFX-API FSR"))
		{
			ExecuteData& execData = *reinterpret_cast<ExecuteData*>(data);

			if (ImGui::BeginCombo("FSR Version", execData.VersionNames[execData.SelectedVersion]))
			{
				for (U64 i = 0; i <= execData.VersionCount; ++i)
				{
					const bool selected = i == execData.SelectedVersion;
					if (ImGui::Selectable(execData.VersionNames[i], selected))
						execData.SelectedVersion = i;
					if (selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			constexpr std::array<const char*, 5> LEVELS = { "Ultra Performance", "Performance", "Balanced", "Quality", "Native AA" };
			if (ImGui::BeginCombo("Quality level", LEVELS.at(4U - static_cast<U8>(execData.Quality))))
			{
				for (FfxApiUpscaleQualityMode i = FFX_UPSCALE_QUALITY_MODE_ULTRA_PERFORMANCE; const char* level : LEVELS)
				{
					const bool selected = i == execData.Quality;
					if (ImGui::Selectable(level, selected))
						execData.Quality = i;
					if (selected)
						ImGui::SetItemDefaultFocus();
					i = static_cast<FfxApiUpscaleQualityMode>(static_cast<U8>(i) - 1U);
				}
				ImGui::EndCombo();
			}

			ImGui::Columns(2, "##sharpness_settings", false);
			{
				ImGui::Text("Sharpness");
			}
			ImGui::NextColumn();
			{
				ImGui::Checkbox("##enable_sharpness", &execData.SharpeningEnabled);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable an additional sharpening pass");
			}
			ImGui::Columns(1);

			if (!execData.SharpeningEnabled)
				ImGui::BeginDisabled(true);
			GUI::InputClamp(0.0f, 1.0f, execData.Sharpness,
				ImGui::InputFloat("##ffxfsr_sharpness", &execData.Sharpness, 0.01f, 0.1f, "%.2f"));
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("The sharpness value between 0 and 1, where 0 is no additional sharpness and 1 is maximum additional sharpness");
			if (!execData.SharpeningEnabled)
				ImGui::EndDisabled();
			ImGui::NewLine();
		}
	}
}