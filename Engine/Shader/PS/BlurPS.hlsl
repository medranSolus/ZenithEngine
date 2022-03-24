#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"
#include "CB/BlurDirection.hlsli"

Texture2D tex : register(t0);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	float2 delta;
	if (cb_vertical)
		delta = float2(0.0f, 1.0f / cb_pbrData.BlurHeight);
	else
		delta = float2(1.0f / cb_pbrData.BlurWidth, 0.0f);

	float3 maxColor = 0.0f;
	float alpha = 0.0f;
	for (int i = -cb_pbrData.BlurRadius; i <= cb_pbrData.BlurRadius; ++i)
	{
		const float4 color = tex.Sample(splr_LM, tc + delta * i);
		alpha += color.a * cb_pbrData.BlurCoefficients[abs(i)];
		maxColor = max(maxColor, color.rgb);
	}
	return float4(maxColor * cb_pbrData.BlurIntensity, alpha);
}