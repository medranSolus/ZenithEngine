#include "GFX/Pipeline/RenderPass/UpscaleFSR1.h"
#include "GFX/Error.h"
#include "GFX/FfxBackendInterface.h"
#include "GUI/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::UpscaleFSR1
{
	ExecuteData::~ExecuteData()
	{
		if (Initialized)
		{
			Settings::RenderSize = Settings::DisplaySize;
			ZE_FFX_CHECK(ffxFsr1ContextDestroy(&Ctx), "Error destroying FSR1 context!");
		}
	}

	static Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, PassExecuteData* passData, const std::vector<PixelFormat>& formats) noexcept
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for FSR1 update formats!");
		return Update(dev, buildData, *static_cast<ExecuteData*>(passData), formats.front());
	}

	static ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData) noexcept
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
		desc.DebugUI = DebugUI;
		return desc;
	}

	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, PixelFormat formatOutput) noexcept
	{
		UInt2 renderSize = CalculateRenderSize(dev, Settings::DisplaySize, UpscalerType::Fsr1, passData.Quality);
		if (renderSize != Settings::RenderSize || passData.DisplaySize != Settings::DisplaySize)
		{
			if (passData.Initialized)
			{
				ZE_FFX_LOG_RET_FAILED_EXPECT(ffxFsr1ContextDestroy(&passData.Ctx), "Error destroying FSR1 context!");
				passData.Initialized = false;
			}
			passData.DisplaySize = Settings::DisplaySize;

			FfxFsr1ContextDescription ctxDesc = {};
			ctxDesc.flags = FFX_FSR1_ENABLE_HIGH_DYNAMIC_RANGE | FFX_FSR1_ENABLE_RCAS;
			ctxDesc.outputFormat = FFX::GetSurfaceFormat(formatOutput);
			ctxDesc.maxRenderSize = { renderSize.X, renderSize.Y };
			ctxDesc.displaySize = { passData.DisplaySize.X, passData.DisplaySize.Y };
			ctxDesc.backendInterface = buildData.FfxInterface;
			ZE_FFX_LOG_RET_FAILED_EXPECT(ffxFsr1ContextCreate(&passData.Ctx, &ctxDesc), "Error creating FSR1 context!");
			passData.Initialized = true;

			Settings::RenderSize = renderSize;
			return UpdateOperation::FrameBufferImpact;
		}
		return UpdateOperation::NoUpdate;
	}

	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatOutput) noexcept
	{
		auto passData = std::make_shared<ExecuteData>();
		auto operation = Update(dev, buildData, *passData, formatOutput);
		if (!operation)
			return std::unexpected(operation.error());
		return passData;
	}

	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept
	{
		ZE_PERF_GUARD("Upscale FSR1");

		Resources ids = *reinterpret_cast<Resources*>(passData.Resources.get());
		ExecuteData& data = *static_cast<ExecuteData*>(passData.ExecData.get());
		const UInt2 inputSize = renderData.Buffers.GetDimmensions(ids.Color);

		ZE_DRAW_TAG_BEGIN(dev, cl, "Upscale FSR1", Pixel(0xC2, 0x32, 0x32));

		FfxFsr1DispatchDescription desc = {};
		desc.commandList = FFX::GetCommandList(cl);
		desc.color = FFX::GetResource(renderData.Buffers, ids.Color, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.output = FFX::GetResource(renderData.Buffers, ids.Output, FFX_RESOURCE_STATE_UNORDERED_ACCESS);
		desc.renderSize = { inputSize.X, inputSize.Y };
		desc.enableSharpening = data.SharpeningEnabled;
		desc.sharpness = data.Sharpness;
		ZE_FFX_LOG_RET_FAILED_EXPECT(ffxFsr1ContextDispatch(&data.Ctx, &desc), "Error performing FSR1!");

		ZE_DRAW_TAG_END(dev, cl);
		return true;
	}

	void DebugUI(PassExecuteData* data) noexcept
	{
		if (ImGui::CollapsingHeader("FSR 1"))
		{
			ExecuteData& execData = *static_cast<ExecuteData*>(data);
			ImGui::Text("Version " ZE_STRINGIFY_VERSION(ZE_DEPAREN(FFX_FSR1_VERSION_MAJOR), ZE_DEPAREN(FFX_FSR1_VERSION_MINOR), ZE_DEPAREN(FFX_FSR1_VERSION_PATCH)));

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