#include "Utils/LightUtils.hlsli"
#include "Utils/Samplers.hlsli"
#include "CBuffer/DirectionalLight.hlsli"
#include "CBuffer/Camera.hlsli"
#include "CBuffer/Bias.hlsli"
#include "CBuffer/ShadowSpace.hlsli"
#include "HDRGammaCB.hlsli"
#include "GBuffer.hlsli"
#define _LIGHT_PS
#include "LightBuffer.hlsli"
#include "DepthBuffer.hlsli"

Texture2D shadowMap : register(t0);

PSOut main(float2 tc : TEXCOORD)
{
	PSOut pso;
	const float3 lightColor = DeleteGammaCorr(cb_lightColor) * cb_lightIntensity;

	const float isSolid = tx_color.Sample(splr_PW, tc).a;
	[branch]
	if (isSolid == 0.0f)
	{
		const float3 position = GetWorldPosition(tc, tx_depth.Sample(splr_PW, tc).x, cb_inverseViewProjection);
		const float3 shadowColor = DeleteGammaCorr(cb_shadowColor);

		// Shadow test (cb_mapSize from BiasPB is bound implicitly from Shadow Mapping Pass since it has to be run always before Lighting Pass)
		const float shadowLevel = 1.0f; // GetShadowLevel(normalize(cb_cameraPos - position), lightDistance, directionToLight, GetShadowUV(position), splr_AB, shadowMap, cb_mapSize);
		if (shadowLevel != 0.0f)
		{
			const float3 normal = DecodeNormal(tx_normal.Sample(splr_PW, tc).rg);
			const float3 directionToLight = -cb_direction;
			pso.color = float4(lerp(shadowColor, GetDiffuse(lightColor, directionToLight, normal), shadowLevel), 0.0f);

			if (shadowLevel > 0.98f)
			{
				const float4 specularData = tx_specularColor.Sample(splr_PW, tc);
				pso.specular = float4(GetSpecular(cb_cameraPos, directionToLight, position, normal,
					pso.color.rgb * specularData.rgb, specularData.a), 0.0f);
			}
			else
				pso.specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		else
		{
			pso.color = float4(-shadowColor, 0.0f);
			pso.specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
		}
	}
	else
	{
		pso.color = float4(lightColor, 0.0f);
		pso.specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	return pso;
}