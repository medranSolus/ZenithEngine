#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"
#include "Utils/Utils.hlsli"
#include "CB/Phong.hlsli"

Texture2D tex : register(t0);
Texture2D normalMap : register(t1);
Texture2D spec : register(t2);
Texture2D parallax : register(t3);

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
	const float3x3 TBN = GetTangentToWorld(worldTan, worldNormal);
	[branch]
	if (cb_material.Flags & FLAG_USE_PARALLAX)
	{
		tc = GetParallaxMapping(tc, normalize(mul(TBN, cameraDir)), cb_material.ParallaxScale, parallax, splr_AR);
		[branch]
		if (tc.x > 1.0f || tc.y > 1.0f || tc.x < 0.0f || tc.y < 0.0f)
			discard;
	}

	PSOut pso;

	pso.color = cb_material.Flags & FLAG_USE_TEXTURE ? tex.Sample(splr_AR, tc) : cb_material.Color;
	clip(pso.color.a - 0.0039f);
	pso.color.a = 0.0f;

	pso.normal = EncodeNormal(cb_material.Flags & FLAG_USE_NORMAL ? GetMappedNormal(TBN, tc, normalMap, splr_AR) : normalize(worldNormal));

	const float4 specular = cb_material.Flags & FLAG_USE_SPECULAR ? spec.Sample(splr_AR, tc) : float4(cb_material.Specular, cb_material.SpecularPower);
	pso.specular = float4(specular.rgb * cb_material.SpecularIntensity,
		GetSampledSpecularPower(cb_material.Flags & FLAG_USE_SPECULAR_POWER_ALPHA ? specular.a : cb_material.SpecularPower));

	return pso;
}