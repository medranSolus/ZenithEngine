#include "UtilsPS.hlsli"
#include "PhongPB.hlsli"

#ifdef _TEX
SamplerState splr : register(s0);
Texture2D tex : register(t0);
#ifdef _TEX_NORMAL
Texture2D normalMap : register(t1);
#ifdef _TEX_PAX
Texture2D parallax : register(t3);
#endif
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
#ifdef _TEX_PAX
	, float3 cameraDir : CAMERADIR
#endif
#endif
#endif
)
{
#ifdef _TEX_NORMAL
	const float3x3 TBN = GetTangentToWorld(worldBitan, worldNormal);
#ifdef _TEX_PAX
	tc = GetParallaxMapping(tc, normalize(mul(TBN, cameraDir)), cb_parallaxScale, parallax, splr);
	if (tc.x > 1.0f || tc.y > 1.0f || tc.x < 0.0f || tc.y < 0.0f)
		discard;
#endif
#endif

	PSOut pso;
#ifdef _TEX
	pso.color = tex.Sample(splr, tc);
#else
	pso.color = cb_materialColor;
#endif
	clip(pso.color.a - 0.0039f);
	pso.color.a = 0.0f;

#ifdef _TEX_NORMAL
	pso.normal = EncodeNormal(GetMappedNormal(TBN, tc, normalMap, splr));
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