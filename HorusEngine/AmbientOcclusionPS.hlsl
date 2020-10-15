#include "UtilsPS.hlsli"
#include "LightUtilsPS.hlsli"
#include "SSAOKernelPB.hlsli"
#include "SSAOOptionsPB.hlsli"
#include "CameraPB.hlsli"

SamplerState splr  : register(s1);

Texture2D normalTex   : register(t5); // RG - normal

Texture2D depthMap    : register(t8);
Texture2D noiseMap    : register(t12); // RG - random vector

float main(float2 tc : TEXCOORD) : SV_TARGET
{
	const float2 sphericNormal = normalTex.Sample(splr, tc).rg;
	if (sphericNormal.x == 0.0f && sphericNormal.y == 0.0f)
		return 1.0f;

	const float3 normal = DecodeNormal(sphericNormal);
	const float3 position = GetWorldPosition(tc, depthMap.Sample(splr, tc).x, cb_inverseViewProjection);
	const float2 noise = noiseMap.Sample(splr, tc * cb_tileDimensions).rg;

	const float3x3 TBN = GetTangentToWorldUNorm(float3(noise, 0.0f), normal);
	float occlusion = 0.0f;
	[unroll]
	for (uint i = 0; i < cb_kernelSize; ++i)
	{
		const float4 samplePos = mul(float4(position + mul(TBN, cb_kernel[i]) * cb_sampleRadius, 1.0f), cb_viewProjection); // From tangent to clip space
		const float2 offset = samplePos.xy / (samplePos.w * 2.0f) + 0.5f; // Perspective divide and transform to 0-1
		const float sampleDepth = GetLinearDepth(depthMap.Sample(splr, float2(offset.x, 1.0f - offset.y)).x, cb_nearClip, cb_farClip) - cb_bias;
		const float rangeCheck = smoothstep(0.0f, 1.0f, cb_sampleRadius / abs(sampleDepth - samplePos.z));
		occlusion += (sampleDepth > samplePos.z ? 1.0f : 0.0f) * rangeCheck;
	}
	return occlusion / cb_kernelSize;
}