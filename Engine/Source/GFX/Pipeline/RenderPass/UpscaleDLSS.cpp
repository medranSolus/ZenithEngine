#include "GFX/pipeline/RenderPass/UpscaleDLSS.h"

namespace ZE::GFX::Pipeline::RenderPass::UpscaleDLSS
{
	static UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats) { return Update(dev, buildData, *reinterpret_cast<ExecuteData*>(passData)); }

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData) { return Initialize(dev, buildData); }

	PassDesc GetDesc() noexcept
	{
		PassDesc desc{ Base(CorePassType::UpscaleDLSS) };
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.Clean = Clean;
		return desc;
	}

	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus)
	{
		syncStatus.SyncMain(dev);
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		NgxInterface* ngx = dev.GetNGX();
		if (ngx)
		{
			if (execData->DlssHandle)
				ngx->FreeFeature(execData->DlssHandle);
			if (execData->NgxParam)
				ngx->FreeParameter(execData->NgxParam);
		}
		delete execData;
	}

	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData)
	{
		NgxInterface* ngx = dev.GetNGX();
		if (ngx)
		{
			UInt2 renderSize = CalculateRenderSize(dev, Settings::DisplaySize, UpscalerType::DLSS, passData.Quality);
			if (renderSize != Settings::RenderSize || passData.DisplaySize != Settings::DisplaySize)
			{
				Settings::RenderSize = renderSize;
				passData.DisplaySize = Settings::DisplaySize;

				passData.NgxParam->Set(NVSDK_NGX_Parameter_Width, renderSize.X);
				passData.NgxParam->Set(NVSDK_NGX_Parameter_Height, renderSize.Y);
				passData.NgxParam->Set(NVSDK_NGX_Parameter_OutWidth, passData.DisplaySize.X);
				passData.NgxParam->Set(NVSDK_NGX_Parameter_OutHeight, passData.DisplaySize.Y);
				passData.NgxParam->Set(NVSDK_NGX_Parameter_PerfQualityValue, passData.Quality);

				if (passData.DlssHandle)
					ngx->FreeFeature(passData.DlssHandle);

				passData.DlssHandle = ngx->CreateFeature(dev, NVSDK_NGX_Feature_SuperSampling, passData.NgxParam);
				ZE_ASSERT(passData.DlssHandle, "Error creating DLSS feature!");
				return UpdateStatus::FrameBufferImpact;
			}
		}
		return UpdateStatus::NoUpdate;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData)
	{
		ExecuteData* passData = nullptr;
		NgxInterface* ngx = dev.GetNGX();
		if (ngx)
		{
			ZE_ASSERT(ngx->IsFeatureAvailable(dev, NVSDK_NGX_Feature_SuperSampling), "DLSS is not available to be run on this system!");
			passData = new ExecuteData;

			passData->NgxParam = ngx->AllocateParameter();
			ZE_ASSERT(passData->NgxParam, "Error allocating DLSS parameter!");

			passData->NgxParam->Set(NVSDK_NGX_Parameter_CreationNodeMask, 1U);
			passData->NgxParam->Set(NVSDK_NGX_Parameter_VisibilityNodeMask, 1U);
			passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags,
				NVSDK_NGX_DLSS_Feature_Flags_IsHDR | NVSDK_NGX_DLSS_Feature_Flags_MVLowRes
				| NVSDK_NGX_DLSS_Feature_Flags_DepthInverted | NVSDK_NGX_DLSS_Feature_Flags_AutoExposure
				| NVSDK_NGX_DLSS_Feature_Flags_DoSharpening);
			passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Enable_Output_Subrects, 0);

			/* Optimization presets
				NVSDK_NGX_DLSS_Hint_Render_Preset_Default -> stick to whatever Nvidia will choose over OTA
				NVSDK_NGX_DLSS_Hint_Render_Preset_A -> intended for Performance/Balanced/Quality, older one, better fighting of ghosting with missing inputs (ex. motion vectors)
				NVSDK_NGX_DLSS_Hint_Render_Preset_B -> intended for Ultra Performance, similar to A
				NVSDK_NGX_DLSS_Hint_Render_Preset_C -> intended for Performance/Balanced/Quality, favors current frame info, good for fast pacing games
				NVSDK_NGX_DLSS_Hint_Render_Preset_D -> intended for Performance/Balanced/Quality, default for this modes, favors image stability
				NVSDK_NGX_DLSS_Hint_Render_Preset_E -> unused
				NVSDK_NGX_DLSS_Hint_Render_Preset_F -> intended for Ultra Performance/DLAA, default for this modes
				NVSDK_NGX_DLSS_Hint_Render_Preset_G -> unused
			*/
			passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_DLAA, NVSDK_NGX_DLSS_Hint_Render_Preset_F);
			passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraQuality, NVSDK_NGX_DLSS_Hint_Render_Preset_Default);
			passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Quality, NVSDK_NGX_DLSS_Hint_Render_Preset_D);
			passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Balanced, NVSDK_NGX_DLSS_Hint_Render_Preset_D);
			passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Performance, NVSDK_NGX_DLSS_Hint_Render_Preset_D);
			passData->NgxParam->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraPerformance, NVSDK_NGX_DLSS_Hint_Render_Preset_F);

			// Some default parameters
			passData->NgxParam->Set(NVSDK_NGX_Parameter_TonemapperType, NVSDK_NGX_TONEMAPPER_REINHARD);
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

			Update(dev, buildData, *passData);
		}
		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Upscale DLSS");
		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();
		const UInt2 inputSize = renderData.Buffers.GetDimmensions(ids.Color);

		ZE_DRAW_TAG_BEGIN(dev, cl, "Upscale DLSS", Pixel(0xAD, 0xFF, 0x2F));

		renderData.Buffers.SetResourceNGX(data.NgxParam, NVSDK_NGX_Parameter_Color, ids.Color);
		renderData.Buffers.SetResourceNGX(data.NgxParam, NVSDK_NGX_Parameter_Depth, ids.Depth);
		renderData.Buffers.SetResourceNGX(data.NgxParam, NVSDK_NGX_Parameter_MotionVectors, ids.MotionVectors);
		renderData.Buffers.SetResourceNGX(data.NgxParam, NVSDK_NGX_Parameter_Output, ids.Output);

		data.NgxParam->Set(NVSDK_NGX_Parameter_Jitter_Offset_X, renderData.DynamicData.JitterCurrent.x);
		data.NgxParam->Set(NVSDK_NGX_Parameter_Jitter_Offset_Y, renderData.DynamicData.JitterCurrent.y);
		data.NgxParam->Set(NVSDK_NGX_Parameter_MV_Scale_X, -Utils::SafeCast<float>(inputSize.X));
		data.NgxParam->Set(NVSDK_NGX_Parameter_MV_Scale_Y, -Utils::SafeCast<float>(inputSize.Y));
		data.NgxParam->Set(NVSDK_NGX_Parameter_Sharpness, data.Sharpness);
		data.NgxParam->Set(NVSDK_NGX_Parameter_Reset, renderData.GraphData.FrameTemporalReset);
		data.NgxParam->Set(NVSDK_NGX_Parameter_FrameTimeDeltaInMsec, Utils::SafeCast<float>(Settings::FrameTime));

		[[maybe_unused]] bool status = dev.GetNGX()->RunFeature(dev, cl, data.DlssHandle, data.NgxParam);
		ZE_ASSERT(status, "Error running DLSS upscaling!");

		ZE_DRAW_TAG_END(dev, cl);
	}
}