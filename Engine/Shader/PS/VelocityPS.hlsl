#include "CommonUtils.hlsli"
#include "Samplers.hlsli"
#include "WorldDataCB.hlsli"

TEX2D(depthMap, 0, 0);
TEX2D(prevDepthMap, 1, 2);

float2 main(float2 tc : TEXCOORD) : SV_TARGET0
{
	// TODO: Jitter cancelation and verify that it's correct method for:
	// a) depth reconstruction
	// b) motion computation
	
	// Position depth reconstruction
	const float3 currentWorldPos = GetWorldPosition(tc, tx_depthMap.Sample(splr_PR, tc).x, cb_worldData.ViewProjectionInverse);
	const float3 prevWorldPos = GetWorldPosition(tc, tx_prevDepthMap.Sample(splr_PR, tc).x, cb_worldData.PrevViewProjectionInverse);
	
	// Get position on screen
	float4 currentViewPos = mul(float4(currentWorldPos, 1.0f), cb_worldData.ViewProjection);
	currentViewPos.xy /= currentViewPos.w;
	float4 prevViewPos = mul(float4(prevWorldPos, 1.0f), cb_worldData.PrevViewProjection);
	prevViewPos.xy /= prevViewPos.w;
	
	return currentViewPos.xy - prevViewPos.xy;
}