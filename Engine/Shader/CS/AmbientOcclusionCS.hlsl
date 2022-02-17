#include "CommonUtils.hlsli"
#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"

// Get tangent space rotation (not normalized)
float3x3 GetTangentToWorldUNorm(const in float3 tan, const in float3 normal)
{
	// Make bitangent again orthogonal to normal (Gramm-Schmidt process)
	const float3 T = normalize(tan - dot(tan, normal) * normal);
	return float3x3(T, cross(normal, T), normal);
}

RWTexture2D<float> ssaoMap   : register(u0);
Texture2D<float2>  normalMap : register(t0);
Texture2D<float>   depthMap  : register(t1);
Texture2D<float2>  noiseMap  : register(t2); // RG - random vector

[numthreads(32, 32, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID)
{
	const float2 sphericNormal = normalMap[dispatchId.xy];
	float ssaoVal = 1.0f;
	if (sphericNormal.x != 0.0f || sphericNormal.y != 0.0f)
	{
		const float3 normal = DecodeNormal(sphericNormal);
		const float2 texCoord = dispatchId.xy / (float2)(cb_pbrData.FrameDimmensions - 1);
		const float3 position = GetWorldPosition(texCoord, depthMap[dispatchId.xy], cb_pbrData.ViewProjectionInverse);
		const float2 noise = noiseMap.SampleLevel(splr_LW, texCoord * cb_pbrData.SsaoNoiseDimmensions, 0);

		const float3x3 TBN = GetTangentToWorldUNorm(normalize(float3(noise, 0.0f)), normal);
		float occlusion = 0.0f;
		uint size = 0;
		[unroll(SSAO_KERNEL_MAX_SIZE)]
		for (uint i = 0; i < cb_pbrData.SsaoKernelSize; ++i)
		{
			const float3 sampleRay = mul(cb_pbrData.SsaoKernel[i], TBN);

			if (dot(normalize(sampleRay), normal) >= 0.15f)
			{
				// From tangent to clip space
				const float4 samplePos = mul(float4(position + sampleRay * cb_pbrData.SsaoSampleRadius, 1.0f), cb_pbrData.ViewProjection);
				// Perspective divide and transform to 0-1
				const float2 offset = samplePos.xy / (samplePos.w * 2.0f) + 0.5f;

				const float sampleDepth = GetLinearDepth(depthMap.SampleLevel(splr_PR, float2(offset.x, 1.0f - offset.y), 0)) + cb_pbrData.SsaoBias;
				const float rangeCheck = smoothstep(0.0f, 1.0f, cb_pbrData.SsaoSampleRadius / abs(sampleDepth - samplePos.z));

				occlusion += ((sampleDepth >= samplePos.z ? 0.0f : 1.0f) * rangeCheck - occlusion) / ++size;
			}
		}
		ssaoVal = pow(1.0f - occlusion, cb_pbrData.SsaoPower);
	}
	ssaoMap[dispatchId.xy] = ssaoVal;
}