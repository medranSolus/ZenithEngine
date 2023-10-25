#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"
#include "Utils/Utils.hlsli"
#include "CB/Phong.hlsli"

#ifdef _USE_TEXTURE
TEX2D(tex, 0, 2);
#endif
#ifdef _USE_NORMAL
TEX2D(normalMap, 1, 2);
#endif
#ifdef _USE_SPECULAR
TEX2D(spec, 2, 2);
#endif
#ifdef _USE_PARALLAX
TEX2D(parallax, 3, 2);
#endif

struct PSOut
{
	float4 color : SV_TARGET0;    // RGB - color, A = 0.0f
	float2 normal : SV_TARGET1;
	float4 specular : SV_TARGET2; // RGB - color, A - power
	float alpha : SV_TARGET3;
};

PSOut main(float3 worldPos : POSITION,
	float3 worldNormal : NORMAL,
	float2 tc : TEXCOORD,
	float3 worldTan : TANGENT,
	float3 cameraDir : CAMERADIR)
{
#if defined(_USE_NORMAL) || defined(_USE_PARALLAX)
	const float3x3 TBN = GetTangentToWorld(worldTan, worldNormal);
#	ifdef _USE_PARALLAX
	tc = GetParallaxMapping(tc, normalize(mul(TBN, cameraDir)), cb_material.ParallaxScale, tx_parallax, splr_AR);
	[branch]
	if (tc.x > 1.0f || tc.y > 1.0f || tc.x < 0.0f || tc.y < 0.0f)
		discard;
#	endif
#endif

	PSOut pso;
	
#ifdef _USE_TEXTURE
	pso.color = tx_tex.Sample(splr_AR, tc);
	clip(pso.color.a - 0.0039f);
	pso.alpha = clamp(pso.color.a, 0.0f, 0.9f);
	pso.color.a = 0.0f;
#else
	pso.color = float4(cb_material.Color.rgb, 0.0f);
	pso.alpha = 0.0f;
#endif

#ifdef _USE_NORMAL
	pso.normal = EncodeNormal(GetMappedNormal(TBN, tc, tx_normalMap, splr_AR));
#else
	pso.normal = EncodeNormal(normalize(worldNormal));
#endif

#ifdef _USE_SPECULAR
	const float4 specularTex = tx_spec.Sample(splr_AR, tc);
	pso.specular = float4(specularTex.rgb * cb_material.SpecularIntensity,
		GetSampledSpecularPower(cb_material.Flags & FLAG_USE_SPECULAR_POWER_ALPHA ? specularTex.a : cb_material.SpecularPower));
#else
	pso.specular = float4(cb_material.Specular * cb_material.SpecularIntensity, GetSampledSpecularPower(cb_material.SpecularPower));
#endif

	return pso;
}