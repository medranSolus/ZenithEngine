#include "UtilsPS.hlsli"

cbuffer PixelBuffer : register(b1)
{
	float3 cb_specularColor;
	float cb_specularIntensity; // The bigger the brighter
	float cb_specularPower;     // The smaller the less focused in one point
#ifdef _TEX
#ifdef _TEX_NORMAL
	float cb_normalMapWeight;
#endif
#ifdef _TEX_SPEC
	bool cb_useSpecularPowerAlpha;
#endif
#else
	float4 cb_materialColor;
#endif
};

#ifdef _TEX
SamplerState splr : register(s0);
Texture2D tex : register(t0);
#ifdef _TEX_NORMAL
Texture2D normalMap : register(t1);
#endif
#ifdef _TEX_SPEC
Texture2D spec : register(t2);
#endif
#endif

struct PSOut
{
	float4 color : SV_TARGET0;    // RGB - color, A = 0.0f
	float2 normal : SV_TARGET1;
	float4 specular : SV_TARGET2; // RGB - color, A - power
};

PSOut main(float3 worldPos : POSITION, float3 worldNormal : NORMAL
#ifdef _TEX
	, float2 tc : TEXCOORD
#ifdef _TEX_NORMAL
	, float3 worldBitan : BITANGENT
#endif
#endif
)
{
	PSOut pso;
#ifdef _TEX
	pso.color = tex.Sample(splr, tc);
#else
	pso.color = cb_materialColor;
#endif
	clip(pso.color.a - 0.0039f);
	pso.color.a = 0.0f;

#ifdef _TEX_NORMAL
	pso.normal = EncodeNormal(lerp(worldPos,
		GetMappedNormal(worldBitan, worldNormal, tc, normalMap, splr), cb_normalMapWeight));
#else
	pso.normal = EncodeNormal(normalize(worldNormal));
#endif
	// Only for double sided objects
	//if (dot(viewNormal, viewPos) >= 0.0f)
	//	viewNormal *= -1.0f;

#ifdef _TEX_SPEC
	const float4 specularTex = spec.Sample(splr, tc);
	pso.specular = float4(specularTex.rgb * cb_specularIntensity,
		cb_useSpecularPowerAlpha ? specularTex.a : cb_specularPower);
#else
	pso.specular = float4(cb_specularColor * cb_specularIntensity, cb_specularPower);
#endif
	return pso;
}