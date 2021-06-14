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

static const uint2 THREAD_COUNT = uint2(32, 32);
groupshared float groupSsao[THREAD_COUNT.x][THREAD_COUNT.y];

[numthreads(THREAD_COUNT.x, THREAD_COUNT.y, 1)]
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

	static const int BEGIN = -3;
	static const uint LAST = 3;
	int s = BEGIN;
	uint u = 1;
	uint count = 0;

	// Vertical blur
	[unroll]
	for (; s < 0; ++s)
		ssaoVal += (groupSsao[threadId.x][abs((int)threadId.y + s)] - ssaoVal) / ++count;
	[unroll]
	for (; u <= LAST; ++u)
	{
		uint offset = threadId.y + u;
		if (offset >= THREAD_COUNT.y)
			offset = threadId.y - u;
		ssaoVal += (groupSsao[threadId.x][offset] - ssaoVal) / ++count;
	}
	groupSsao[threadId.x][threadId.y] = ssaoVal;
	AllMemoryBarrierWithGroupSync();

	// Horizontal blur
	[unroll]
	for (s = BEGIN; s < 0; ++s)
		ssaoVal += (groupSsao[abs((int)threadId.x + s)][threadId.y] - ssaoVal) / ++count;
	[unroll]
	for (u = 1; u <= LAST; ++u)
	{
		uint offset = threadId.x + u;
		if (offset > THREAD_COUNT.x)
			offset = threadId.x - u;
		ssaoVal += (groupSsao[offset][threadId.y] - ssaoVal) / ++count;
	}
	ssaoMap[dispatchId.xy] = ssaoVal;
}