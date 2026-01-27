#ifdef _ZE_FILMIC_VDR
#	include "CB/TonemapParamsVDR.hlsli"
#elif defined(_ZE_AGX)
#	include "CB/TonemapParamsAgX.hlsli"
#elif defined(_ZE_REINHARD) || defined(_ZE_REINHARD_LUMA) || defined(_ZE_REINHARD_JODIE)
#	include "CB/TonemapParamsReinhard.hlsli"
#elif defined(_ZE_REINHARD_EXTENDED) || defined(_ZE_REINHARD_LUMA_WHITE)
#	include "CB/TonemapParamsReinhardX.hlsli"
#else
#	include "CB/TonemapParams.hlsli"
#endif
#include "Utils/Tonemap.hlsli"
#include "Samplers.hlsli"

TEX2D(frame, 0, 0);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	// Radiance W / (m^2 * sr)
	const float4 hdrColor = tx_frame.Sample(splr_PR, tc);
	
	// TODO: Implement:
	//       - http://cs.brown.edu/courses/cs129/results/proj5/njooma/ as HDR image processing (requires bilateral filter http://people.csail.mit.edu/sparis/bf_course/)
	//       - ACES 2.0 when optimized version for GPUs become available
	//       - real camera response https://64.github.io/tonemapping/ (requires LUT)
#ifdef _ZE_EXPOSURE
	const float3 mapped = GetTonemapExposure(hdrColor.rgb, ct_params.Exposure);
#elif defined(_ZE_RBDH)
	const float3 mapped = GetTonemapRBDH(hdrColor.rgb, ct_params.Exposure);
#elif defined(_ZE_FILMIC_HABLE)
	const float3 mapped = GetFilmicHable(hdrColor.rgb, ct_params.Exposure);
#elif defined(_ZE_ACES)
	const float3 mapped = GetACES(hdrColor.rgb, ct_params.Exposure);
#elif defined(_ZE_ACES_NAUTILUS)
	const float3 mapped = GetACESNautilus(hdrColor.rgb, ct_params.Exposure);
#elif defined(_ZE_KHRONOS_PBR)
	const float3 mapped = GetKhronosPBRNeutral(hdrColor.rgb, ct_params.Exposure);
#elif defined(_ZE_REINHARD)
	const float3 mapped = GetReinhard(hdrColor.rgb, ct_params.Exposure, ct_params.Offset);
#elif defined(_ZE_REINHARD_LUMA)
	const float3 mapped = GetReinhardLuma(hdrColor.rgb, ct_params.Exposure, ct_params.Offset);
#elif defined(_ZE_REINHARD_JODIE)
	const float3 mapped = GetReinhardJodie(hdrColor.rgb, ct_params.Exposure, ct_params.Offset);
#elif defined(_ZE_REINHARD_EXTENDED)
	const float3 mapped = GetReinhard(hdrColor.rgb, ct_params.Exposure, ct_params.Offset, ct_params.MaxWhite);
#elif defined(_ZE_REINHARD_LUMA_WHITE)
	const float3 mapped = GetReinhardLuma(hdrColor.rgb, ct_params.Exposure, ct_params.Offset, ct_params.MaxWhite);
#elif defined(_ZE_FILMIC_VDR)
	const float3 mapped = GetFilmicVDR(hdrColor.rgb, ct_params.Exposure, ct_params.Contrast, ct_params.B, ct_params.C, ct_params.Shoulder);
#elif defined(_ZE_AGX)
	const float3 mapped = GetAgX(hdrColor.rgb, ct_params.Exposure, ct_params.Saturation, ct_params.Contrast, ct_params.MidContrast);
#else
	// No tonemapping
	const float3 mapped = saturate(hdrColor.rgb * ct_params.Exposure);
#endif
	
	return float4(mapped, hdrColor.a);
}