#include "GFX/pipeline/RenderPass/UpscaleDLSS.h"
#include "GFX/External/InterfaceStorage.h"
#include "Data/Camera.h"
#include "GUI/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::UpscaleDLSS
{
	ExecuteData::~ExecuteData()
	{
		auto ngx = External::InterfaceStorage::GetConnectionNGX();
		if (ngx)
		{
			if (DlssHandle)
			{
				Settings::RenderSize = Settings::DisplaySize;
				ngx->FreeFeature(DlssHandle);
			}
			if (NgxParam)
				ngx->FreeParameter(NgxParam);
			External::InterfaceStorage::ReleaseConnectionNGX();
		}
	}

	static Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, PassExecuteData* passData, const std::vector<PixelFormat>& formats) noexcept
	{
		return Update(dev, buildData, *static_cast<ExecuteData*>(passData));
	}

	static ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, PassInitData* initData) noexcept
	{
		return Initialize(dev, buildData);
	}

	PassDesc GetDesc() noexcept
	{
		PassDesc desc{ Base(CorePassType::UpscaleDLSS) };
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.DebugUI = DebugUI;
		return desc;
	}

	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData) noexcept
	{
		auto ngx = External::InterfaceStorage::GetConnectionNGX();
		if (!ngx)
			return std::unexpected(ZE_NGX_ERROR(NVSDK_NGX_Result_FAIL_NotInitialized));

		ZE_ASSERT(passData.Quality != NVSDK_NGX_PerfQuality_Value_UltraQuality, "DLSS ultra quality setting currently unsuported!");

		UInt2 renderSize = CalculateRenderSize(dev, Settings::DisplaySize, UpscalerType::DLSS, passData.Quality);
		bool sizeChanged = renderSize != Settings::RenderSize || passData.DisplaySize != Settings::DisplaySize;
		if (sizeChanged || passData.PresetUpdate)
		{
			if (passData.DlssHandle)
				ngx->FreeFeature(passData.DlssHandle);
			passData.DisplaySize = Settings::DisplaySize;
			passData.PresetUpdate = false;

			passData.NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_DLAA, passData.QualityPresets.at(5));
			passData.NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraQuality, passData.QualityPresets.at(4));
			passData.NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Quality, passData.QualityPresets.at(3));
			passData.NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Balanced, passData.QualityPresets.at(2));
			passData.NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Performance, passData.QualityPresets.at(1));
			passData.NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraPerformance, passData.QualityPresets.at(0));

			passData.NgxParam->Set(NVSDK_NGX_Parameter_Width, renderSize.X);
			passData.NgxParam->Set(NVSDK_NGX_Parameter_Height, renderSize.Y);
			passData.NgxParam->Set(NVSDK_NGX_Parameter_OutWidth, passData.DisplaySize.X);
			passData.NgxParam->Set(NVSDK_NGX_Parameter_OutHeight, passData.DisplaySize.Y);
			passData.NgxParam->Set(NVSDK_NGX_Parameter_PerfQualityValue, passData.Quality);
			ZE_NGX_LOG_RET_FAILED_EXPECT(ngx->CreateFeature(dev, NVSDK_NGX_Feature_SuperSampling, passData.NgxParam, passData.DlssHandle), "Error creating DLSS feature!");

			Settings::RenderSize = renderSize;
			return sizeChanged ? UpdateOperation::FrameBufferImpact : UpdateOperation::InternalOnly;
		}
		return UpdateOperation::NoUpdate;
	}

	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData) noexcept
	{
		auto ngx = External::InterfaceStorage::CreateConnectionNGX(dev);
		if (!ngx)
			return std::unexpected(ZE_NGX_ERROR(NVSDK_NGX_Result_Fail));

		if (!ngx->IsFeatureAvailable(dev, NVSDK_NGX_Feature_SuperSampling))
		{
			Status ret = ZE_NGX_ERROR(NVSDK_NGX_Result_FAIL_FeatureNotSupported);
			ZE_CODE_ERROR(ret, "DLSS is not supported on this system!");
			return std::unexpected(ret);
		}

		auto passData = std::make_shared<ExecuteData>();
		ZE_NGX_LOG_RET_FAILED_EXPECT(ngx->AllocateParameter(passData->NgxParam), "Error allocating DLSS parameter!");

		passData->NgxParam->Set(NVSDK_NGX_Parameter_CreationNodeMask, 1U);
		passData->NgxParam->Set(NVSDK_NGX_Parameter_VisibilityNodeMask, 1U);
		passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags,
			NVSDK_NGX_DLSS_Feature_Flags_IsHDR | NVSDK_NGX_DLSS_Feature_Flags_MVLowRes
			| NVSDK_NGX_DLSS_Feature_Flags_DepthInverted | NVSDK_NGX_DLSS_Feature_Flags_AutoExposure);
		passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Enable_Output_Subrects, false);

		// Optimization presets
		passData->QualityPresets =
		{
			NVSDK_NGX_DLSS_Hint_Render_Preset_F,
			NVSDK_NGX_DLSS_Hint_Render_Preset_K,
			NVSDK_NGX_DLSS_Hint_Render_Preset_K,
			NVSDK_NGX_DLSS_Hint_Render_Preset_K,
			NVSDK_NGX_DLSS_Hint_Render_Preset_Default,
			NVSDK_NGX_DLSS_Hint_Render_Preset_F,
		};

		// Some default parameters
		passData->NgxParam->Set(NVSDK_NGX_Parameter_TonemapperType, NVSDK_NGX_TONEMAPPER_ACES);
		passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSSMode, NVSDK_NGX_DLSS_Mode_DLSS);
		passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Pre_Exposure, 1.0f);
		passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Exposure_Scale, 1.0f);
		passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Input_Color_Subrect_Base_X, 0U);
		passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Input_Color_Subrect_Base_Y, 0U);
		passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Input_Depth_Subrect_Base_X, 0U);
		passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Input_Depth_Subrect_Base_Y, 0U);
		passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Input_MV_SubrectBase_X, 0U);
		passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Input_MV_SubrectBase_Y, 0U);
		passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Input_Translucency_SubrectBase_X, 0U);
		passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Input_Translucency_SubrectBase_Y, 0U);
		passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_SubrectBase_X, 0U);
		passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_SubrectBase_Y, 0U);
		passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Output_Subrect_Base_X, 0U);
		passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Output_Subrect_Base_Y, 0U);
		passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Indicator_Invert_X_Axis, 0U);
		passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Indicator_Invert_Y_Axis, 0U);

		auto operation = Update(dev, buildData, *passData);
		if (!operation)
			return std::unexpected(operation.error());
		return passData;
	}

	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept
	{
		auto ngx = External::InterfaceStorage::GetConnectionNGX();
		if (!ngx)
			return std::unexpected(ZE_NGX_ERROR(NVSDK_NGX_Result_FAIL_NotInitialized));

		ZE_PERF_GUARD("Upscale DLSS");
		Resources ids = *reinterpret_cast<Resources*>(passData.Resources.get());
		ExecuteData& data = *static_cast<ExecuteData*>(passData.ExecData.get());
		const UInt2 inputSize = renderData.Buffers.GetDimmensions(ids.Color);

		ZE_DRAW_TAG_BEGIN(dev, cl, "Upscale DLSS", Pixel(0xAD, 0xFF, 0x2F));

		renderData.Buffers.SetResourceNGX(data.NgxParam, NVSDK_NGX_Parameter_Color, ids.Color);
		renderData.Buffers.SetResourceNGX(data.NgxParam, NVSDK_NGX_Parameter_Depth, ids.Depth);
		renderData.Buffers.SetResourceNGX(data.NgxParam, NVSDK_NGX_Parameter_MotionVectors, ids.MotionVectors);
		renderData.Buffers.SetResourceNGX(data.NgxParam, NVSDK_NGX_Parameter_Output, ids.Output);

		data.NgxParam->Set(NVSDK_NGX_Parameter_Jitter_Offset_X, Data::GetUnitPixelJitterX(renderData.DynamicData.JitterCurrent.x, Settings::RenderSize.X));
		data.NgxParam->Set(NVSDK_NGX_Parameter_Jitter_Offset_Y, Data::GetUnitPixelJitterY(renderData.DynamicData.JitterCurrent.y, Settings::RenderSize.Y));
		data.NgxParam->Set(NVSDK_NGX_Parameter_MV_Scale_X, -Utils::SafeCast<float>(inputSize.X));
		data.NgxParam->Set(NVSDK_NGX_Parameter_MV_Scale_Y, -Utils::SafeCast<float>(inputSize.Y));
		data.NgxParam->Set(NVSDK_NGX_Parameter_Reset, renderData.GraphData.FrameTemporalReset);
		data.NgxParam->Set(NVSDK_NGX_Parameter_FrameTimeDeltaInMsec, Utils::SafeCast<float>(Settings::FrameTime));

		ZE_NGX_LOG_RET_FAILED_EXPECT(ngx->RunFeature(dev, cl, data.DlssHandle, data.NgxParam), "Error running DLSS upscaling!");
		cl.RestoreExternalState(dev);

		ZE_DRAW_TAG_END(dev, cl);
		return true;
	}

	void DebugUI(PassExecuteData* data) noexcept
	{
		if (ImGui::CollapsingHeader("DLSS"))
		{
			ExecuteData& execData = *static_cast<ExecuteData*>(data);

			ImGui::Text("Version 310.6.0 (built with)");

			ImGui::Columns(2, "##dlss_settings", false);
			{
				auto getQualityString = [](NVSDK_NGX_PerfQuality_Value quality) noexcept -> const char*
					{
						switch (quality)
						{
						case NVSDK_NGX_PerfQuality_Value_MaxPerf:
							return "Performance";
						case NVSDK_NGX_PerfQuality_Value_Balanced:
							return "Balanced";
						case NVSDK_NGX_PerfQuality_Value_MaxQuality:
							return "Quality";
						case NVSDK_NGX_PerfQuality_Value_UltraPerformance:
							return "Ultra Performance";
						case NVSDK_NGX_PerfQuality_Value_UltraQuality:
							return "Ultra Quality";
						default:
						case NVSDK_NGX_PerfQuality_Value_DLAA:
							return "DLAA";
						}
					};
				if (ImGui::BeginCombo("Quality level", getQualityString(execData.Quality)))
				{
					constexpr std::array<NVSDK_NGX_PerfQuality_Value, 5> LEVELS =
					{
						NVSDK_NGX_PerfQuality_Value_UltraPerformance,
						NVSDK_NGX_PerfQuality_Value_MaxPerf,
						NVSDK_NGX_PerfQuality_Value_Balanced,
						NVSDK_NGX_PerfQuality_Value_MaxQuality,
						NVSDK_NGX_PerfQuality_Value_DLAA
					};
					for (auto level : LEVELS)
					{
						const bool selected = level == execData.Quality;
						if (ImGui::Selectable(getQualityString(level), selected))
							execData.Quality = level;
						if (selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}
			ImGui::NextColumn();
			{
				auto getPresetString = [](NVSDK_NGX_DLSS_Hint_Render_Preset preset) noexcept -> const char*
					{
						switch (preset)
						{
						default:
							ZE_ENUM_UNHANDLED();
						case NVSDK_NGX_DLSS_Hint_Render_Preset_Default:
							return "Default";
						case NVSDK_NGX_DLSS_Hint_Render_Preset_F:
							return "Preset F";
						case NVSDK_NGX_DLSS_Hint_Render_Preset_J:
							return "Preset J";
						case NVSDK_NGX_DLSS_Hint_Render_Preset_K:
							return "Preset K";
						}
					};
				auto& currentPreset = execData.QualityPresets.at(static_cast<U8>(execData.Quality));
				if (ImGui::BeginCombo("Quality preset", getPresetString(currentPreset)))
				{
					auto getPresetInfoString = [](NVSDK_NGX_DLSS_Hint_Render_Preset preset) noexcept -> const char*
						{
							switch (preset)
							{
							default:
								ZE_ENUM_UNHANDLED();
							case NVSDK_NGX_DLSS_Hint_Render_Preset_Default:
								return "Default behavior";
							case NVSDK_NGX_DLSS_Hint_Render_Preset_F:
								return "Intended for Ultra Perf/DLAA modes. The default preset for Ultra Perf";
							case NVSDK_NGX_DLSS_Hint_Render_Preset_J:
								return "Similar to preset K. Preset J might exhibit slightly less ghosting at the cost of extra flickering. Preset K is generally recommended over preset J";
							case NVSDK_NGX_DLSS_Hint_Render_Preset_K:
								return "Default preset for DLAA/Perf/Balanced/Quality modes that is transformer based. Best image quality preset at a higher performance cost";
							}
						};
					constexpr std::array<NVSDK_NGX_DLSS_Hint_Render_Preset, 4> PRESETS =
					{
						NVSDK_NGX_DLSS_Hint_Render_Preset_Default,
						NVSDK_NGX_DLSS_Hint_Render_Preset_F,
						NVSDK_NGX_DLSS_Hint_Render_Preset_J,
						NVSDK_NGX_DLSS_Hint_Render_Preset_K,
					};
					for (auto preset : PRESETS)
					{
						const bool selected = preset == currentPreset;
						if (ImGui::Selectable(getPresetString(preset), selected))
						{
							currentPreset = preset;
							execData.PresetUpdate = true;
						}
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip(getPresetInfoString(preset));
						if (selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}
			ImGui::Columns(1);

			ImGui::NewLine();
		}
	}
}