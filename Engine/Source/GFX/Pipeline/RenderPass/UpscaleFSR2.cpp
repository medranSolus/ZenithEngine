#include "GFX/Pipeline/RenderPass/UpscaleFSR2.h"
#include "GFX/FfxBackendInterface.h"
#include "Data/Camera.h"
#include "GUI/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::UpscaleFSR2
{
	static UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats) { return Update(dev, buildData, *reinterpret_cast<ExecuteData*>(passData)); }

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData) { return Initialize(dev, buildData); }

	static void MessageHandler(FfxMsgType type, const wchar_t* msg) noexcept
	{
		switch (type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case FFX_MESSAGE_TYPE_ERROR:
			Logger::Error(Utils::ToUTF8(msg));
			break;
		case FFX_MESSAGE_TYPE_WARNING:
			Logger::Warning(Utils::ToUTF8(msg));
			break;
		}
	}

	PassDesc GetDesc() noexcept
	{
		PassDesc desc{ Base(CorePassType::UpscaleFSR2) };
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
		Settings::RenderSize = Settings::DisplaySize;
		syncStatus.SyncMain(dev);
		ZE_FFX_ENABLE();
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		ZE_FFX_CHECK(ffxFsr2ContextDestroy(&execData->Ctx), "Error destroying FSR2 context!");
		delete execData;
	}

	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, bool firstUpdate)
	{
		UInt2 renderSize = CalculateRenderSize(dev, Settings::DisplaySize, UpscalerType::Fsr2, passData.Quality);
		if (renderSize != Settings::RenderSize || passData.DisplaySize != Settings::DisplaySize)
		{
			ZE_FFX_ENABLE();
			Settings::RenderSize = renderSize;
			passData.DisplaySize = Settings::DisplaySize;

			if (!firstUpdate)
			{
				buildData.SyncStatus.SyncMain(dev);
				ZE_FFX_CHECK(ffxFsr2ContextDestroy(&passData.Ctx), "Error destroying FSR2 context!");
			}
			FfxFsr2ContextDescription ctxDesc = {};
			ctxDesc.flags = FFX_FSR2_ENABLE_DEPTH_INVERTED | FFX_FSR2_ENABLE_DEPTH_INFINITE
				| FFX_FSR2_ENABLE_AUTO_EXPOSURE | FFX_FSR2_ENABLE_HIGH_DYNAMIC_RANGE
				| (_ZE_DEBUG_GFX_API ? FFX_FSR2_ENABLE_DEBUG_CHECKING : 0);
			ctxDesc.maxRenderSize = { renderSize.X, renderSize.Y };
			ctxDesc.displaySize = { passData.DisplaySize.X, passData.DisplaySize.Y };
			ctxDesc.backendInterface = buildData.FfxInterface;
			ctxDesc.fpMessage = MessageHandler;
			ZE_FFX_THROW_FAILED(ffxFsr2ContextCreate(&passData.Ctx, &ctxDesc), "Error creating FSR2 context!");
			return UpdateStatus::FrameBufferImpact;
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
		ZE_FFX_ENABLE();
		ZE_PERF_GUARD("Upscale FSR2");

		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();
		const UInt2 inputSize = renderData.Buffers.GetDimmensions(ids.Color);

		ZE_DRAW_TAG_BEGIN(dev, cl, "Upscale FSR2", Pixel(0xB2, 0x22, 0x22));

		FfxFsr2DispatchDescription desc = {};
		desc.commandList = FFX::GetCommandList(cl);
		desc.color = FFX::GetResource(renderData.Buffers, ids.Color, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.depth = FFX::GetResource(renderData.Buffers, ids.Depth, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.motionVectors = FFX::GetResource(renderData.Buffers, ids.MotionVectors, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.exposure.resource = nullptr;
		desc.reactive = FFX::GetResource(renderData.Buffers, ids.ReactiveMask, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.transparencyAndComposition.resource = nullptr; // Alpha value for special surfaces (reflections, animated textures, etc.), add when needed
		desc.output = FFX::GetResource(renderData.Buffers, ids.Output, FFX_RESOURCE_STATE_UNORDERED_ACCESS);
		desc.jitterOffset.x = Data::GetUnitPixelJitterX(renderData.DynamicData.JitterCurrent.x, Settings::RenderSize.X);
		desc.jitterOffset.y = Data::GetUnitPixelJitterY(renderData.DynamicData.JitterCurrent.y, Settings::RenderSize.Y);
		desc.motionVectorScale.x = -Utils::SafeCast<float>(inputSize.X);
		desc.motionVectorScale.y = -Utils::SafeCast<float>(inputSize.Y);
		desc.renderSize = { inputSize.X, inputSize.Y };
		desc.enableSharpening = data.SharpeningEnabled;
		desc.frameTimeDelta = Utils::SafeCast<float>(Settings::FrameTime);
		desc.sharpness = data.Sharpness;
		desc.preExposure = 1.0f;
		desc.reset = renderData.GraphData.FrameTemporalReset;
		desc.cameraNear = FLT_MAX;
		desc.cameraFar = renderData.DynamicData.NearClip;
		desc.cameraFovAngleVertical = Settings::Data.get<Data::Camera>(renderData.GraphData.CurrentCamera).Projection.FOV;
		desc.viewSpaceToMetersFactor = 1.0f;
		desc.enableAutoReactive = false;
		desc.colorOpaqueOnly.resource = nullptr;
		desc.autoTcThreshold = 0.05f; // Setting this value too small will cause visual instability. Larger values can cause ghosting
		desc.autoTcScale = 1.0f; // Smaller values will increase stability at hard edges of translucent objects
		desc.autoReactiveScale = 5.00f; // Larger values result in more reactive pixels
		desc.autoReactiveMax = 0.90f; // Maximum value reactivity can reach
		ZE_FFX_THROW_FAILED(ffxFsr2ContextDispatch(&data.Ctx, &desc), "Error performing FSR2!");

		ZE_DRAW_TAG_END(dev, cl);
	}

	void DebugUI(void* data) noexcept
	{
		if (ImGui::CollapsingHeader("FSR 2"))
		{
			ExecuteData& execData = *reinterpret_cast<ExecuteData*>(data);

			ImGui::Text("Version " ZE_STRINGIFY_VERSION(ZE_DEPAREN(FFX_FSR2_VERSION_MAJOR), ZE_DEPAREN(FFX_FSR2_VERSION_MINOR), ZE_DEPAREN(FFX_FSR2_VERSION_PATCH)));
#if _ZE_USE_FFX_API_FSR_SHADERS
			ImGui::SameLine();
			ImGui::Text(" (Shaders version " ZE_STRINGIFY_VERSION(_ZE_FSR2_SHADERS_VERSION_MAJOR, _ZE_FSR2_SHADERS_VERSION_MINOR, _ZE_FSR2_SHADERS_VERSION_PATCH) " from FFX API)");
#endif

			constexpr std::array<const char*, 4> LEVELS = { "Ultra Performance", "Performance", "Balanced", "Quality" };
			if (ImGui::BeginCombo("Quality level", LEVELS.at(4U - static_cast<U8>(execData.Quality))))
			{
				for (FfxFsr2QualityMode i = FFX_FSR2_QUALITY_MODE_ULTRA_PERFORMANCE; const char* level : LEVELS)
				{
					const bool selected = i == execData.Quality;
					if (ImGui::Selectable(level, selected))
						execData.Quality = i;
					if (selected)
						ImGui::SetItemDefaultFocus();
					i = static_cast<FfxFsr2QualityMode>(static_cast<U8>(i) - 1U);
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
				ImGui::InputFloat("##fsr2_sharpness", &execData.Sharpness, 0.01f, 0.1f, "%.2f"));
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("The sharpness value between 0 and 1, where 0 is no additional sharpness and 1 is maximum additional sharpness");
			if (!execData.SharpeningEnabled)
				ImGui::EndDisabled();
			ImGui::NewLine();
		}
	}
}