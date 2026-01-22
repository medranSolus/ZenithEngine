#include "GFX/Pipeline/RenderPass/TonemapLPM.h"
#include "GFX/FfxBackendInterface.h"
#include "GUI/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::TonemapLPM
{
	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData) { return Initialize(dev, buildData); }

	PassDesc GetDesc() noexcept
	{
		PassDesc desc{ Base(CorePassType::TonemapLPM) };
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Clean = Clean;
		desc.DebugUI = DebugUI;
		return desc;
	}

	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus)
	{
		syncStatus.SyncMain(dev);
		syncStatus.SyncCompute(dev);
		ZE_FFX_ENABLE();
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		ZE_FFX_CHECK(ffxLpmContextDestroy(&execData->Ctx), "Error destroying LPM context!");
		delete execData;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData)
	{
		ZE_FFX_ENABLE();
		ExecuteData* passData = new ExecuteData;

		FfxLpmContextDescription ctxDesc = {};
		ctxDesc.flags = 0;
		ctxDesc.backendInterface = buildData.FfxInterface;
		ZE_FFX_THROW_FAILED(ffxLpmContextCreate(&passData->Ctx, &ctxDesc), "Error creating LPM context!");

		return passData;
	}

	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_FFX_ENABLE();
		ZE_PERF_GUARD("TonemapLPM");
		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();

		ZE_DRAW_TAG_BEGIN(dev, cl, "TonemapLPM", PixelVal::White);

		FfxLpmDispatchDescription dispatchDesc = {};
		dispatchDesc.commandList = FFX::GetCommandList(cl);
		dispatchDesc.inputColor = FFX::GetResource(renderData.Buffers, ids.Scene, FFX_RESOURCE_STATE_COMPUTE_READ);
		dispatchDesc.outputColor = FFX::GetResource(renderData.Buffers, ids.RenderTarget, FFX_RESOURCE_STATE_UNORDERED_ACCESS);
		dispatchDesc.shoulder = data.Shoulder;
		dispatchDesc.softGap = data.SoftGap;
		dispatchDesc.hdrMax = data.HdrMax;
		dispatchDesc.lpmExposure = data.Exposure;
		dispatchDesc.contrast = data.Contrast;
		dispatchDesc.shoulderContrast = data.ShoulderContrast;
		dispatchDesc.saturation[0] = data.Saturation.x;
		dispatchDesc.saturation[1] = data.Saturation.y;
		dispatchDesc.saturation[2] = data.Saturation.z;
		dispatchDesc.crosstalk[0] = data.CrossTalk.x;
		dispatchDesc.crosstalk[1] = data.CrossTalk.y;
		dispatchDesc.crosstalk[2] = data.CrossTalk.z;
		dispatchDesc.colorSpace = data.ColorSpace;
		dispatchDesc.displayMode = data.DisplayMode;

		const DisplayProperties& displayProps = dev.GetDisplayProperties();
		dispatchDesc.displayRedPrimary[0] = displayProps.RedPrimary.x;
		dispatchDesc.displayRedPrimary[1] = displayProps.RedPrimary.y;
		dispatchDesc.displayGreenPrimary[0] = displayProps.GreenPrimary.x;
		dispatchDesc.displayGreenPrimary[1] = displayProps.GreenPrimary.y;
		dispatchDesc.displayBluePrimary[0] = displayProps.BluePrimary.x;
		dispatchDesc.displayBluePrimary[1] = displayProps.BluePrimary.y;
		dispatchDesc.displayWhitePoint[0] = displayProps.WhitePoint.x;
		dispatchDesc.displayWhitePoint[1] = displayProps.WhitePoint.y;
		dispatchDesc.displayMinLuminance = displayProps.MinLuminance;
		dispatchDesc.displayMaxLuminance = displayProps.MaxLuminance;
		ZE_FFX_THROW_FAILED(ffxLpmContextDispatch(&data.Ctx, &dispatchDesc), "Error performing LPM!");

		ZE_DRAW_TAG_END(dev, cl);
		return true;
	}

	void DebugUI(void* data) noexcept
	{
		if (ImGui::CollapsingHeader("Luma Preserving Mapper"))
		{
			ExecuteData& execData = *reinterpret_cast<ExecuteData*>(data);

			ImGui::Text("Version " ZE_STRINGIFY_VERSION(ZE_DEPAREN(FFX_LPM_VERSION_MAJOR), ZE_DEPAREN(FFX_LPM_VERSION_MINOR), ZE_DEPAREN(FFX_LPM_VERSION_PATCH)));

			constexpr std::array<const char*, 5> MODES = { "LDR", "HDR10 PQ", "HDR10 scRGB", "FreeSync Premium Pro PQ", "FreeSync Premium Pro scRGB" };
			if (ImGui::BeginCombo("Display mode", MODES.at(static_cast<U8>(execData.DisplayMode))))
			{
				for (U8 i = 0; const char* mode : MODES)
				{
					const bool selected = static_cast<FfxLpmDisplayMode>(i) == execData.DisplayMode;
					if (ImGui::Selectable(mode, selected))
						execData.DisplayMode = static_cast<FfxLpmDisplayMode>(i);
					if (ImGui::IsItemHovered())
					{
						switch (static_cast<FfxLpmDisplayMode>(i))
						{
						case FfxLpmDisplayMode::FFX_LPM_DISPLAYMODE_LDR:
							ImGui::SetTooltip("Targets low or standard dynamic range monitor using 8-bit back buffer");
							break;
						case FfxLpmDisplayMode::FFX_LPM_DISPLAYMODE_HDR10_2084:
							ImGui::SetTooltip("Targets HDR10 perceptual quantizer (PQ) transfer function using 10-bit backbuffer");
							break;
						case FfxLpmDisplayMode::FFX_LPM_DISPLAYMODE_HDR10_SCRGB:
							ImGui::SetTooltip("Targets HDR10 linear output with no transfer function using 16-bit backbuffer");
							break;
						case FfxLpmDisplayMode::FFX_LPM_DISPLAYMODE_FSHDR_2084:
							ImGui::SetTooltip("Targets FreeSync Premium Pro HDR through PQ transfer function using 10-bit backbuffer");
							break;
						case FfxLpmDisplayMode::FFX_LPM_DISPLAYMODE_FSHDR_SCRGB:
							ImGui::SetTooltip("Targets FreeSync Premium Pro HDR linear output with no transfer function using 16-bit backbuffer");
							break;
						default:
							break;
						}
					}
					if (selected)
						ImGui::SetItemDefaultFocus();
					++i;
				}
				ImGui::EndCombo();
			}

			constexpr std::array<const char*, 4> SPACES = { "Rec.709", "P3", "Rec.2020", "Custom display" };
			if (ImGui::BeginCombo("Color space", SPACES.at(static_cast<U8>(execData.ColorSpace))))
			{
				for (U8 i = 0; const char* space : SPACES)
				{
					const bool selected = static_cast<FfxLpmColorSpace>(i) == execData.ColorSpace;
					if (ImGui::Selectable(space, selected))
						execData.ColorSpace = static_cast<FfxLpmColorSpace>(i);
					if (ImGui::IsItemHovered())
					{
						switch (static_cast<FfxLpmColorSpace>(i))
						{
						case FfxLpmColorSpace::FFX_LPM_ColorSpace_REC709:
							ImGui::SetTooltip("Uses Rec.709 color primaries used for LDR, HDR10 scRGB and FreeSync Premium Pro scRGB display modes");
							break;
						case FfxLpmColorSpace::FFX_LPM_ColorSpace_P3:
							ImGui::SetTooltip("Uses P3 color primaries");
							break;
						case FfxLpmColorSpace::FFX_LPM_ColorSpace_REC2020:
							ImGui::SetTooltip("Uses Rec.2020 color primaries used for HDR10 PQ and FreeSync Premium Pro PQ display modes");
							break;
						case FfxLpmColorSpace::FFX_LPM_ColorSapce_Display:
							ImGui::SetTooltip("Uses custom primaries queried from display");
							break;
						default:
							break;
						}
					}
					if (selected)
						ImGui::SetItemDefaultFocus();
					++i;
				}
				ImGui::EndCombo();
			}

			ImGui::Columns(2, nullptr, false);
			{
				ImGui::Text("HDR Max");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(6.0f, 2048.0f, execData.HdrMax,
					ImGui::InputFloat("##lpm_hdr_max", &execData.HdrMax, 1.0f, 10.0f, "%.1f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Maximum input value");

				ImGui::Text("Contrast");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.0f, 1.0f, execData.Contrast,
					ImGui::InputFloat("##lpm_contrast", &execData.Contrast, 0.1f, 0.0f, "%.1f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("0 - no extra contrast, 1 - maximum contrast");

				ImGui::Text("Soft Gap");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.0f, 0.5f, execData.SoftGap,
					ImGui::InputFloat("##lpm_soft_gap", &execData.SoftGap, 0.01f, 0.1f, "%.2f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Controls how much feather region in out-of-gamut mapping, 0 means clipping");

				ImGui::Text("Saturation");
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::SliderFloat3("##lpm_saturation", reinterpret_cast<float*>(&execData.Saturation), -1.0f, 1.0f, "%.2f", ImGuiSliderFlags_ColorMarkers);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("A per channel adjustment, set to below 0 to decrease, 0 for no change and over 0 to increase");
			}
			ImGui::NextColumn();
			{
				ImGui::Text("Exposure");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(1.0f, FLT_MAX, execData.Exposure,
					ImGui::InputFloat("##lpm_exposure", &execData.Exposure, 0.1f, 1.0f, "%.1f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Number of stops between HDR Max and 18%c mid-level on input", '%');

				ImGui::Text("Shoulder Contrast");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(1.0f, 1.5f, execData.ShoulderContrast,
					ImGui::InputFloat("##lpm_shoulder_contrast", &execData.ShoulderContrast, 0.01f, 0.1f, "%.2f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Shoulder shaping power, 1 means no change (fast path)");

				ImGui::NewLine();
				ImGui::Checkbox("Shoulder", &execData.Shoulder);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Use optional extra shoulder tuning (no effect if shoulder contrast is 1)");

				ImGui::Text("Crosstalk");
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::SliderFloat3("##lpm_crosstalk", reinterpret_cast<float*>(&execData.CrossTalk), 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_ColorMarkers);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Crosstalk scaling for over-exposure color shaping. One channel must be 1, the rest can be below or equal to 1 but not 0. Lenghtnes colors path to white by walking across gamut");
			}
			ImGui::Columns(1);
			ImGui::NewLine();
		}
	}
}