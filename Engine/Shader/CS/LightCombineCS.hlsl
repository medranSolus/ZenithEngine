#include "CBuffer/LightAmbient.hlsli"
#include "HDRGammaCB.hlsli"

RWTexture2D<float4> frame : register(u0);

Texture2D<float4> colorTex    : register(t4);  // RGB - color, A = 0.0f
Texture2D<float4> lightingMap : register(t9);  // RGB - light color
Texture2D<float4> specularMap : register(t10); // RGB - specular color
Texture2D<float4> ssaoMap     : register(t11); // R   - ambient occlusion value

// 32 threads = one WARP
[numthreads(32, 32, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
	const float4 srgb = colorTex[dispatchThreadId.xy];
	const float3 color = DeleteGammaCorr(srgb.rgb);
	const float ssao = ssaoMap[dispatchThreadId.xy].r;
	const float3 diffuse = ssao * color * (DeleteGammaCorr(cb_ambientLight) + abs(lightingMap[dispatchThreadId.xy].rgb));

	frame[dispatchThreadId.xy] = float4(diffuse + specularMap[dispatchThreadId.xy].rgb, srgb.a);
}