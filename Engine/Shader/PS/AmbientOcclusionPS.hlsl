#include "Utils/Utils.hlsli"
#include "Utils/LightUtils.hlsli"
#include "Utils/Samplers.hlsli"
#include "CBuffer/SSAOKernel.hlsli"
#include "CBuffer/Camera.hlsli"

Texture2D normalTex   : register(t5); // RG - normal

Texture2D depthMap    : register(t8);
Texture2D noiseMap    : register(t12); // RG - random vector

float main(float2 tc : TEXCOORD) : SV_TARGET
{
	const float2 sphericNormal = normalTex.Sample(splr_PR, tc).rg;
	if (sphericNormal.x == 0.0f && sphericNormal.y == 0.0f)
		return 1.0f;

	const float3 normal = DecodeNormal(sphericNormal);
	const float3 position = GetWorldPosition(tc, depthMap.Sample(splr_PR, tc).x, cb_inverseViewProjection);
	const float2 noise = noiseMap.Sample(splr_LW, tc * cb_tileDimensions).rg;

	const float3x3 TBN = GetTangentToWorldUNorm(normalize(float3(noise, 0.0f)), normal);
	float occlusion = 0.0f;
	uint size = cb_kernelSize;
	[unroll]
	for (uint i = 0; i < size; ++i)
	{
		const float3 sampleRay = mul(cb_kernel[i], TBN);
		if (dot(normalize(sampleRay), normal) < 0.15f)
		{
			--size;
			continue;
		}
		const float4 samplePos = mul(float4(position + sampleRay * cb_sampleRadius, 1.0f), cb_viewProjection); // From tangent to clip space
		const float2 offset = samplePos.xy / (samplePos.w * 2.0f) + 0.5f; // Perspective divide and transform to 0-1
		const float sampleDepth = GetLinearDepth(depthMap.Sample(splr_PR, float2(offset.x, 1.0f - offset.y)).x, cb_nearClip, cb_farClip) + cb_bias;
		const float rangeCheck = smoothstep(0.0f, 1.0f, cb_sampleRadius / abs(sampleDepth - samplePos.z));
		occlusion += (sampleDepth >= samplePos.z ? 0.0f : 1.0f) * rangeCheck;
	}
	return pow(1.0f - occlusion / size, cb_ssaoPower);
}