#include "CB/DebugView.hlsli"
#include "GBufferUtils.hlsli"
#include "Samplers.hlsli"
#include "SettingsDataCB.hlsli"

TEX2D(frame, 0, 0);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	if (ct_mode.ViewMode == ZE_DEBUG_VIEW_MODE_SSAO)
	{
		const float ssao = asuint(tx_frame[tc * cb_settingsData.RenderSize].r) / 255.0f;
		return float4(ssao, ssao, ssao, 0.0f);
	}
	else
	{
		float4 data = tx_frame.Sample(splr_PR, tc);
	
		switch (ct_mode.ViewMode)
		{
			case ZE_DEBUG_VIEW_MODE_DEPTH:
				data = float4(data.rrr, 0.0f);
				break;
			case ZE_DEBUG_VIEW_MODE_NORMALS:
				data = float4(data.rg, 0.0f, 0.0f);
				break;
			case ZE_DEBUG_VIEW_MODE_METAL:
				data = float4(GetMetalness(ExtractMaterialParams(data)).rrr, 0.0f);
				break;
			case ZE_DEBUG_VIEW_MODE_ROUGH:
				data = float4(GetRoughness(ExtractMaterialParams(data)).rrr, 0.0f);
				break;
			case ZE_DEBUG_VIEW_MODE_MOTION:
				data = float4(data.rg, 0.0f, 0.0f);
				break;
			case ZE_DEBUG_VIEW_MODE_REACTIVE:
				data = float4(data.rrr, 0.0f);
				break;
			default:
			case ZE_DEBUG_VIEW_MODE_ALBEDO:
			case ZE_DEBUG_VIEW_MODE_DIRECT_LIGHT:
			case ZE_DEBUG_VIEW_MODE_SSR:
			case ZE_DEBUG_VIEW_MODE_RENDERED_SCENE:
			case ZE_DEBUG_VIEW_MODE_UPSCALED_SCENE:
			case ZE_DEBUG_VIEW_MODE_OUTLINE:
				break;
		}
		return data;
	}
}