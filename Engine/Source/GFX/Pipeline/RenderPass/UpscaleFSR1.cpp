#include "GFX/Pipeline/RenderPass/UpscaleFSR1.h"
#include "GFX/FfxBackendInterface.h"
#include "GUI/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::UpscaleFSR1
{
	static UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats)
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for FSR1 update formats!");
		return Update(dev, buildData, *reinterpret_cast<ExecuteData*>(passData), formats.front());
	}

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData)
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for FSR1 initialization formats!");
		return Initialize(dev, buildData, formats.front());
	}

	PassDesc GetDesc(PixelFormat formatOutput) noexcept
	{
		PassDesc desc{ Base(CorePassType::UpscaleFSR1) };
		desc.InitializeFormats.emplace_back(formatOutput);
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
		ZE_FFX_CHECK(ffxFsr1ContextDestroy(&execData->Ctx), "Error destroying FSR1 context!");
		delete execData;
	}

	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, PixelFormat formatOutput, bool firstUpdate)
	{
		UInt2 renderSize = CalculateRenderSize(dev, Settings::DisplaySize, UpscalerType::Fsr1, passData.Quality);
		if (renderSize != Settings::RenderSize || passData.DisplaySize != Settings::DisplaySize)
		{
			ZE_FFX_ENABLE();
			Settings::RenderSize = renderSize;
			passData.DisplaySize = Settings::DisplaySize;

			if (!firstUpdate)
			{
				buildData.SyncStatus.SyncMain(dev);
				ZE_FFX_CHECK(ffxFsr1ContextDestroy(&passData.Ctx), "Error destroying FSR1 context!");
			}
			FfxFsr1ContextDescription ctxDesc = {};
			ctxDesc.flags = FFX_FSR1_ENABLE_HIGH_DYNAMIC_RANGE | FFX_FSR1_ENABLE_RCAS;
			ctxDesc.outputFormat = FFX::GetSurfaceFormat(formatOutput);
			ctxDesc.maxRenderSize = { renderSize.X, renderSize.Y };
			ctxDesc.displaySize = { passData.DisplaySize.X, passData.DisplaySize.Y };
			ctxDesc.backendInterface = buildData.FfxInterface;
			ZE_FFX_THROW_FAILED(ffxFsr1ContextCreate(&passData.Ctx, &ctxDesc), "Error creating FSR1 context!");
			return UpdateStatus::FrameBufferImpact;
		}
		return UpdateStatus::NoUpdate;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatOutput)
	{
		ExecuteData* passData = new ExecuteData;
		Update(dev, buildData, *passData, formatOutput, true);
		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_FFX_ENABLE();
		ZE_PERF_GUARD("Upscale FSR1");

		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();
		const UInt2 inputSize = renderData.Buffers.GetDimmensions(ids.Color);

		ZE_DRAW_TAG_BEGIN(dev, cl, "Upscale FSR1", Pixel(0xC2, 0x32, 0x32));

		FfxFsr1DispatchDescription desc = {};
		desc.commandList = FFX::GetCommandList(cl);
		desc.color = FFX::GetResource(renderData.Buffers, ids.Color, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.output = FFX::GetResource(renderData.Buffers, ids.Output, FFX_RESOURCE_STATE_UNORDERED_ACCESS);
		desc.renderSize = { inputSize.X, inputSize.Y };
		desc.enableSharpening = data.SharpeningEnabled;
		desc.sharpness = data.Sharpness;
		ZE_FFX_THROW_FAILED(ffxFsr1ContextDispatch(&data.Ctx, &desc), "Error performing FSR1!");

		ZE_DRAW_TAG_END(dev, cl);
	}

	void DebugUI(void* data) noexcept
	{
		if (ImGui::CollapsingHeader("FSR 1"))
		{
			ExecuteData& execData = *reinterpret_cast<ExecuteData*>(data);

			constexpr std::array<const char*, 4> LEVELS = { "Performance", "Balanced", "Quality", "Ultra Quality" };
			if (ImGui::BeginCombo("Quality level", LEVELS.at(3U - static_cast<U8>(execData.Quality))))
			{
				for (FfxFsr1QualityMode i = FFX_FSR1_QUALITY_MODE_PERFORMANCE; const char* level : LEVELS)
				{
					const bool selected = i == execData.Quality;
					if (ImGui::Selectable(level, selected))
						execData.Quality = i;
					if (selected)
						ImGui::SetItemDefaultFocus();
					i = static_cast<FfxFsr1QualityMode>(static_cast<U8>(i) - 1U);
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
				ImGui::InputFloat("##fsr_sharpness", &execData.Sharpness, 0.01f, 0.1f, "%.2f"));
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("The sharpness value between 0 and 1, where 0 is no additional sharpness and 1 is maximum additional sharpness");
			if (!execData.SharpeningEnabled)
				ImGui::EndDisabled();
			ImGui::NewLine();
		}
	}
}