#include "Samplers.hlsli"
#include "WorldDataCB.hlsli"
#include "PBRDataCB.hlsli"
#include "Utils/Utils.hlsli"
#include "CB/Phong.hlsli"

#ifdef _ZE_USE_TEXTURE
TEX2D(tex, 0, 2);
#endif
#ifdef _ZE_USE_NORMAL
TEX2D(normalMap, 1, 2);
#endif
#ifdef _ZE_USE_SPECULAR
TEX2D(spec, 2, 2);
#endif
#ifdef _ZE_USE_PARALLAX
TEX2D(parallax, 3, 2);
#endif

struct PSOut
{
	float4 color : SV_TARGET0;    // RGB - color, A = 0.0f
	float2 normal : SV_TARGET1;
	float4 specular : SV_TARGET2; // RGB - color, A - power
#ifdef _ZE_OUTPUT_MOTION
	float2 motion : SV_TARGET3;
#	ifdef _ZE_OUTPUT_REACTIVE
	float reactive : SV_TARGET4;
#	endif
#elif defined(_ZE_OUTPUT_REACTIVE)
	float reactive : SV_TARGET3;
#endif
};

PSOut main(float3 worldPos : POSITION,
	float3 worldNormal : NORMAL,
	float2 tc : TEXCOORD,
	float3 worldTan : TANGENT,
	float3 cameraDir : CAMERADIR
#ifdef _ZE_OUTPUT_MOTION
	, float4 prevPos : PREVPOSITION,
	float4 currentPos : CURRPOSITION
#endif
)
{
#if defined(_ZE_USE_NORMAL) || defined(_ZE_USE_PARALLAX)
	const float3x3 TBN = GetTangentToWorld(worldTan, worldNormal);
#	ifdef _ZE_USE_PARALLAX
	tc = GetParallaxMapping(tc, normalize(mul(TBN, cameraDir)), cb_material.ParallaxScale, tx_parallax, splr_AR, cb_pbrData.MipBias);
	[branch]
	if (tc.x > 1.0f || tc.y > 1.0f || tc.x < 0.0f || tc.y < 0.0f)
		discard;
#	endif
#endif

	PSOut pso;
	
#ifdef _ZE_USE_TEXTURE
	pso.color = tx_tex.SampleBias(splr_AR, tc, cb_pbrData.MipBias);
	clip(pso.color.a - 0.0039f);
	pso.color.a = 1.0f;
#else
	pso.color = float4(cb_material.Color.rgb, 1.0f);
#endif
	

#ifdef _ZE_USE_NORMAL
	pso.normal = EncodeNormal(GetMappedNormal(TBN, tc, tx_normalMap, splr_AR, cb_pbrData.MipBias));
#else
	pso.normal = EncodeNormal(normalize(worldNormal));
#endif

#ifdef _ZE_USE_SPECULAR
	const float4 specularTex = tx_spec.SampleBias(splr_AR, tc, cb_pbrData.MipBias);
	pso.specular = float4(specularTex.rgb * cb_material.SpecularIntensity,
		GetSampledSpecularPower(cb_material.Flags & ZE_PHONG_USE_SPECULAR_POWER_ALPHA ? specularTex.a : cb_material.SpecularPower));
#else
	pso.specular = float4(cb_material.Specular * cb_material.SpecularIntensity, GetSampledSpecularPower(cb_material.SpecularPower));
#endif
	
#ifdef _ZE_OUTPUT_REACTIVE
	pso.reactive = clamp(abs(pso.color.a - 1.0f), 0.0f, 0.9f);
#endif
#ifdef _ZE_OUTPUT_MOTION
	// Motion vectors in NDC space
	float2 prev = prevPos.xy / prevPos.w;
	float2 current = currentPos.xy / currentPos.w;
	// Cancel jitter
	prev -= cb_worldData.JitterPrev;
	current -= cb_worldData.JitterCurrent;
	// To UV motion
	pso.motion = prev - current;
#endif

	return pso;
}