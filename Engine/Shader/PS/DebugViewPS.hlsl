#include "CB/DebugView.hlsli"
#include "GBufferUtils.hlsli"
#include "Samplers.hlsli"
#include "SettingsDataCB.hlsli"

#ifdef _ZE_UINT_INPUT
TEXTURE_EX(frame, Texture2D<uint4>, 0, 0);
#else
TEX2D(frame, 0, 0);
#endif

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
#ifdef _ZE_UINT_INPUT
	uint4 data = tx_frame.Load(int3(tc * cb_settingsData.RenderSize, 0));
	float4 output = float4(data);
	
	if (ct_mode.ViewMode == ZE_DEBUG_VIEW_MODE_SSAO)
	{
		const float ssao = data.r / 255.0f;
		output = float4(ssao, ssao, ssao, 0.0f);
	}
	return output;
#else
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
#endif
}