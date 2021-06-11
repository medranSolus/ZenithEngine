#include "CBuffer/LightAmbient.hlsli"
#include "HDRGammaCB.hlsli"
#include "GBuffer.hlsli"
#include "LightBuffer.hlsli"
#include "SSAOMap.hlsli"

RWTexture2D<float4> frame : register(u0);

// 32 threads = one WARP
[numthreads(32, 32, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
	const float4 srgb = tx_color[dispatchThreadId.xy];
	const float3 color = DeleteGammaCorr(srgb.rgb);
	const float ssao = tx_ssao[dispatchThreadId.xy];
	const float3 diffuse = ssao * color * (DeleteGammaCorr(cb_ambientLight) + abs(tx_lighting[dispatchThreadId.xy].rgb));

	frame[dispatchThreadId.xy] = float4(diffuse + tx_specular[dispatchThreadId.xy].rgb, srgb.a);
}