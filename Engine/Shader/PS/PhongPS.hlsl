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
	PSOut pso;

	[branch]
	if (cb_material.Flags & FLAG_USE_NORMAL || cb_material.Flags & FLAG_USE_PARALLAX)
	{
		const float3x3 TBN = GetTangentToWorld(worldTan, worldNormal);
		[branch]
		if (cb_material.Flags & FLAG_USE_PARALLAX)
		{
			tc = GetParallaxMapping(tc, normalize(mul(TBN, cameraDir)), cb_material.ParallaxScale, parallax, splr_AW);
			if (tc.x > 1.0f || tc.y > 1.0f || tc.x < 0.0f || tc.y < 0.0f)
				discard;
		}
		[branch]
		if (cb_material.Flags & FLAG_USE_NORMAL)
			pso.normal = EncodeNormal(GetMappedNormal(TBN, tc, normalMap, splr_AW));
	}

	[branch]
	if (cb_material.Flags & FLAG_USE_TEXTURE)
		pso.color = tex.Sample(splr_AW, tc);
	else
		pso.color = cb_material.Color;

	clip(pso.color.a - 0.0039f);
	pso.color.a = 0.0f;

	if ((cb_material.Flags & FLAG_USE_NORMAL) == 0)
		pso.normal = EncodeNormal(normalize(worldNormal));

	if (cb_material.Flags & FLAG_USE_SPECULAR)
	{
		const float4 specularTex = spec.Sample(splr_AW, tc);
		pso.specular = float4(specularTex.rgb * cb_material.SpecularIntensity,
			GetSampledSpecularPower(cb_material.Flags & FLAG_USE_SPECULAR_POWER_ALPHA ?
				specularTex.a : cb_material.SpecularPower));
	}
	else
		pso.specular = float4(cb_material.Specular * cb_material.SpecularIntensity,
			GetSampledSpecularPower(cb_material.SpecularPower));

	return pso;
}