#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"
#include "Utils/Utils.hlsli"
#include "CB/Phong.hlsli"

#ifdef _USE_TEXTURE
Texture2D tex : register(t0);
#endif
#ifdef _USE_NORMAL
Texture2D normalMap : register(t1);
#endif
#ifdef _USE_SPECULAR
Texture2D spec : register(t2);
#endif
#ifdef _USE_PARALLAX
Texture2D parallax : register(t3);
#endif

struct PSOut
{
	float4 color : SV_TARGET0;    // RGB - color, A = 0.0f
	float2 normal : SV_TARGET1;
	float4 specular : SV_TARGET2; // RGB - color, A - power
};

PSOut main(float3 worldPos : POSITION,
	float3 worldNormal : NORMAL,
	float2 tc : TEXCOORD,
	float3 worldTan : TANGENT,
	float3 cameraDir : CAMERADIR)
{
#if defined(_USE_NORMAL) || defined(_USE_PARALLAX)
	const float3x3 TBN = GetTangentToWorld(worldTan, worldNormal);
#ifdef _USE_PARALLAX
	tc = GetParallaxMapping(tc, normalize(mul(TBN, cameraDir)), cb_material.ParallaxScale, parallax, splr_AR);
	[branch]
	if (tc.x > 1.0f || tc.y > 1.0f || tc.x < 0.0f || tc.y < 0.0f)
		discard;
#endif
#endif

	PSOut pso;

#ifdef _USE_TEXTURE
	pso.color = tex.Sample(splr_AR, tc);
	clip(pso.color.a - 0.0039f);
	pso.color.a = 0.0f;
#else
	pso.color = float4(cb_material.Color.rgb, 0.0f);
#endif

#ifdef _USE_NORMAL
	pso.normal = EncodeNormal(GetMappedNormal(TBN, tc, normalMap, splr_AR));
#else
	pso.normal = EncodeNormal(normalize(worldNormal));
#endif

#ifdef _USE_SPECULAR
	const float4 specularTex = spec.Sample(splr_AR, tc);
	pso.specular = float4(specularTex.rgb * cb_material.SpecularIntensity,
		GetSampledSpecularPower(cb_material.Flags & FLAG_USE_SPECULAR_POWER_ALPHA ? specularTex.a : cb_material.SpecularPower));
#else
	pso.specular = float4(cb_material.Specular * cb_material.SpecularIntensity, GetSampledSpecularPower(cb_material.SpecularPower));
#endif

	return pso;
}