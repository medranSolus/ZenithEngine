#include "CommonUtils.hlsli"
#include "CBuffer/SSAOKernel.hlsli"
#include "CameraCB.hlsli"
#include "GBuffer.hlsli"
#include "DepthBuffer.hlsli"
#include "Samplers.hlsli"

// Get tangent space rotation (not normalized)
float3x3 GetTangentToWorldUNorm(const in float3 tan, const in float3 normal)
{
	// Make bitangent again orthogonal to normal (Gramm-Schmidt process)
	const float3 T = normalize(tan - dot(tan, normal) * normal);
	return float3x3(T, cross(normal, T), normal);
}

// Convert depth value from logarithmic depth space to linear view space
float GetLinearDepth(const in float depth, uniform float nearClip, uniform float farClip)
{
	return nearClip * farClip / (farClip + depth * (nearClip - farClip));
}

RWTexture2D<float> ssaoMap : register(u0);
Texture2D<float2> noiseMap : register(t24); // RG - random vector

groupshared float groupSsao[32][32];

[numthreads(32, 32, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID, uint3 threadId : SV_GroupThreadID)
{
	const float2 sphericNormal = tx_normal[dispatchId.xy];
	float ssaoVal;
	if (sphericNormal.x == 0.0f && sphericNormal.y == 0.0f)
		ssaoVal = 1.0f;
	else
	{
		const float3 normal = DecodeNormal(sphericNormal);
		const float2 texCoord = dispatchId.xy / (float2)cb_frameBounds;
		const float3 position = GetWorldPosition(texCoord, tx_depth[dispatchId.xy], cb_inverseViewProjection);
		const float2 noise = noiseMap.SampleLevel(splr_LW, texCoord * cb_noiseTileDimensions, 0);

		const float3x3 TBN = GetTangentToWorldUNorm(normalize(float3(noise, 0.0f)), normal);
		float occlusion = 0.0f;
		uint size = 0;
		[unroll]
		for (uint i = 0; i < cb_kernelSize; ++i)
		{
			const float3 sampleRay = mul(cb_kernel[i], TBN);
			if (dot(normalize(sampleRay), normal) >= 0.15f)
			{
				const float4 samplePos = mul(float4(position + sampleRay * cb_sampleRadius, 1.0f), cb_viewProjection); // From tangent to clip space
				const float2 offset = samplePos.xy / (samplePos.w * 2.0f) + 0.5f; // Perspective divide and transform to 0-1
				const float sampleDepth = GetLinearDepth(tx_depth.SampleLevel(splr_PR, float2(offset.x, 1.0f - offset.y), 0), cb_nearClip, cb_farClip) + cb_bias;
				const float rangeCheck = smoothstep(0.0f, 1.0f, cb_sampleRadius / abs(sampleDepth - samplePos.z));
				occlusion += ((sampleDepth >= samplePos.z ? 0.0f : 1.0f) * rangeCheck - occlusion) / ++size;
			}
		}
		ssaoVal = pow(1.0f - occlusion, cb_ssaoPower);
	}
	groupSsao[threadId.x][threadId.y] = ssaoVal;
	ssaoMap[dispatchId.xy] = ssaoVal;
	AllMemoryBarrierWithGroupSync();

	//float2 delta;
	//if (cb_vertical)
	//	delta = float2(0.0f, 0.125f / cb_noiseTileDimensions.y);
	//else
	//	delta = float2(0.25f / cb_noiseTileDimensions.x, 0.0f);

	//static const int RANGE = 3;
	//float result = 0.0f;
	//[unroll]
	//for (int i = -RANGE; i < RANGE; ++i)
	//	result += tx_ssao.Sample(splr_LR, tc + delta * i).r;
	//return result / (RANGE * 2);
}