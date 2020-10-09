#include "UtilsPS.hlsli"
#include "BiasPB.hlsli"
#ifdef _TEX_PAX
#include "ShadowParallaxPB.hlsli"
#endif

#ifdef _TEX
SamplerState splr : register(s0);
Texture2D tex : register(t0);
#ifdef _TEX_PAX
//Texture2D normalMap : register(t1);
Texture2D parallax : register(t3);
#endif
#endif

float main(float3 lightToVertex : VECTOR
#ifdef _TEX
	, float2 tc : TEXCOORD
#ifdef _TEX_PAX
	, float3 worldNormal : NORMAL
	, float3 worldBitan : BITANGENT
	, float3 cameraDir : CAMERADIR
#endif
#endif
) : SV_TARGET
{
#ifdef _TEX
#ifdef _TEX_PAX
	const float3x3 TBN = GetTangentToWorld(worldBitan, worldNormal);
	tc = GetParallaxMapping(tc, normalize(mul(TBN, cameraDir)), cb_parallaxScale, parallax, splr);
	if (tc.x > 1.0f || tc.y > 1.0f || tc.x < 0.0f || tc.y < 0.0f)
		discard;
#endif
	clip(tex.Sample(splr, tc).a - 0.0039f);
#endif
#ifdef _TEX_PAX
	//const float3 normal = GetMappedNormal(TBN, tc, normalMap, splr).rgb;
	//float depth = 1.0f;
	//if (dot(normal, -lightToVertex) <= 0)
	//	depth = GetParallaxDepth(tc, normalize(mul(TBN, -lightToVertex)), parallax, splr);
	//lightToVertex -= normal * depth;
#endif

	return length(lightToVertex) + cb_bias;
}