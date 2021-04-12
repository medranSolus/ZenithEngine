#include "Utils/UtilsPS.hlsli"
#include "Utils/SamplersPS.hlsli"
#include "CBuffer/BiasPB.hlsli"
#include "CBuffer/ShadowPB.hlsli"
#ifdef _TEX_PAX
#include "CBuffer/ShadowParallaxPB.hlsli"
#endif

#ifdef _TEX
Texture2D tex : register(t0);
#ifdef _TEX_PAX
Texture2D normalMap : register(t1);
Texture2D parallax : register(t3);
#endif
#endif

float main(float3 worldPos : POSITION, float3 worldNormal : NORMAL
#ifdef _TEX
	, float2 tc : TEXCOORD
#ifdef _TEX_PAX
	, float3 worldBitan : BITANGENT
	, float3 cameraDir : CAMERADIR
#endif
#endif
) : SV_TARGET
{
#ifdef _TEX
#ifdef _TEX_PAX
	const float3x3 TBN = GetTangentToWorld(worldBitan, worldNormal);
	tc = GetParallaxMapping(tc, normalize(mul(TBN, cameraDir)), cb_parallaxScale, parallax, splr_AW);
	if (tc.x > 1.0f || tc.y > 1.0f || tc.x < 0.0f || tc.y < 0.0f)
		discard;
#endif
	clip(tex.Sample(splr_AW, tc).a - 0.0039f);
#endif
	float3 lightToVertex = worldPos - cb_lightPos;
#ifdef _TEX_PAX
	const float3 normal = GetMappedNormal(TBN, tc, normalMap, splr_PW).rgb;
	float depth = 0.0f;
	//if (dot(normal, lightToVertex) <= 0)
	//	depth = GetParallaxDepth(tc, normalize(mul(TBN, -lightToVertex)), parallax, splr_AW);
	lightToVertex -= normal * (depth + cb_normalOffset);
#else
	lightToVertex -= cb_normalOffset * normalize(worldNormal);
#endif

	return length(lightToVertex) + cb_bias;
}