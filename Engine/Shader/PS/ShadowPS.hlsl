#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"
#include "Utils/Utils.hlsli"
#include "CB/Shadow.hlsli"

Texture2D tex : register(t0);
Texture2D normalMap : register(t1);
Texture2D parallax : register(t3);

float main(float3 worldPos : POSITION,
	float3 worldNormal : NORMAL,
	float2 tc : TEXCOORD,
	float3 worldBitan : BITANGENT,
	float3 cameraDir : CAMERADIR) : SV_TARGET
{
	float depth = 0.0f;
	float3 lightToVertex = worldPos - cb_lightPos;

	if (cb_materialFlags & FLAG_USE_TEXTURE)
	{
		if (cb_materialFlags & (FLAG_USE_PARALLAX | FLAG_USE_NORMAL))
		{
			const float3x3 TBN = GetTangentToWorld(worldBitan, worldNormal);
			tc = GetParallaxMapping(tc, normalize(mul(TBN, cameraDir)), cb_parallaxScale, parallax, splr_AW);
			if (tc.x > 1.0f || tc.y > 1.0f || tc.x < 0.0f || tc.y < 0.0f)
				discard;

			const float3 normal = GetMappedNormal(TBN, tc, normalMap, splr_PW).rgb;
			//if (dot(normal, lightToVertex) <= 0)
			//	depth = GetParallaxDepth(tc, normalize(mul(TBN, -lightToVertex)), parallax, splr_AW);
			lightToVertex -= normal * (depth + cb_pbrData.ShadowNormalOffset);
		}
		else
			lightToVertex -= cb_pbrData.ShadowNormalOffset * normalize(worldNormal);

		clip(tex.Sample(splr_AW, tc).a - 0.0039f);
	}

	return length(lightToVertex) + cb_pbrData.ShadowBias;
}