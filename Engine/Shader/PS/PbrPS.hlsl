#include "GBufferUtils.hlsli"
#include "Samplers.hlsli"
#include "WorldDataCB.hlsli"
#include "PbrDataCB.hlsli"
#include "Utils/Utils.hlsli"
#include "CB/Pbr.hlsli"
#include "Tex/PbrInputs.hlsli"

/* List of permutations:
 *
 * _ZE_TRANSPARENT - texture albedo alpha value may contain cut-out segments
 * _ZE_USE_PARALLAX - use parallax mapping
 * _ZE_OUTPUT_REACTIVE - write to reactive mask for FSR
 * _ZE_OUTPUT_MOTION - compute motion vectors to velocity buffer
 */

struct PSOut
{
	CodedNormalGB normal : SV_TARGET0;
	AlbedoGB albedo : SV_TARGET1;
	PackedMaterialGB materialParams : SV_TARGET2;
#ifdef _ZE_OUTPUT_MOTION
	MotionGB motion : SV_TARGET3;
#	ifdef _ZE_OUTPUT_REACTIVE
	ReactiveGB reactive : SV_TARGET4;
#	endif
#elif defined(_ZE_OUTPUT_REACTIVE)
	ReactiveGB reactive : SV_TARGET3;
#endif
};

PSOut main(float3 worldPos : POSITION,
	float3 worldNormal : NORMAL,
	float2 tc : TEXCOORD,
	float4 worldTan : TANGENTPACK,
	float3 cameraDir : CAMERADIR
#ifdef _ZE_OUTPUT_MOTION
	, float4 prevPos : PREVPOSITION,
	float4 currentPos : CURRPOSITION
#endif
)
{
	float3 normal;
#ifndef _ZE_USE_PARALLAX
	if (cb_material.Flags & ZE_PBR_USE_NORMAL_TEX)
#endif
	{
		const float3x3 TBN = GetTangentToWorld(worldTan, worldNormal);
#ifdef _ZE_USE_PARALLAX
		tc = GetParallaxMapping(tc, normalize(mul(TBN, cameraDir)), cb_material.ParallaxScale, tx_height, splr_AR, cb_pbrData.MipBias);
		[branch]
		if (tc.x > 1.0f || tc.y > 1.0f || tc.x < 0.0f || tc.y < 0.0f)
			discard;
#endif
		normal = GetMappedNormal(TBN, tc, tx_normalMap, splr_AR, cb_pbrData.MipBias);
	}
#ifndef _ZE_USE_PARALLAX
	else
		normal = normalize(worldNormal);
#endif
	
	float4 albedo;
	if (cb_material.Flags & ZE_PBR_USE_ALBEDO_TEX)
	{
		albedo = tx_albedo.SampleBias(splr_AR, tc, cb_pbrData.MipBias);
#ifdef _ZE_TRANSPARENT
		clip(albedo.a - 0.5f);
#endif
	}
	else
		albedo = cb_material.Albedo;
	
	float metalness;
	if (cb_material.Flags & ZE_PBR_USE_METAL_TEX)
		metalness = tx_metalness.SampleBias(splr_AR, tc, cb_pbrData.MipBias).r;
	else
		metalness = cb_material.Metalness;
	
	float roughness;
	if (cb_material.Flags & ZE_PBR_USE_ROUGH_TEX)
		roughness = tx_roughness.SampleBias(splr_AR, tc, cb_pbrData.MipBias).r;
	else
		roughness = cb_material.Roughness;
	
	// Write all outputs
	PSOut pso;
	pso.normal = EncodeNormal(normal);
	pso.albedo = float4(albedo.rgb, 1.0f);
	pso.materialParams = PackMaterialParams(metalness, roughness);
	
#ifdef _ZE_OUTPUT_REACTIVE
	pso.reactive = clamp(abs(pso.albedo.a - 1.0f), 0.0f, cb_pbrData.ReactiveMaskClamp);
#endif
#ifdef _ZE_OUTPUT_MOTION
	// Motion vectors in NDC space
	float2 prev = prevPos.xy / prevPos.w;
	float2 current = currentPos.xy / currentPos.w;
	// Cancel jitter
	prev -= cb_worldData.JitterPrev;
	current -= cb_worldData.JitterCurrent;
	// To UV motion
	pso.motion = (current - prev) * float2(0.5f, -0.5f);
#endif

	return pso;
}