#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"
#include "CB/BlurDirection.hlsli"

Texture2D tex : register(t0);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	float2 delta;
	if (cb_vertical)
		delta = float2(0.0f, 1.0f / cb_pbrData.Blur.Height);
	else
		delta = float2(1.0f / cb_pbrData.Blur.Width, 0.0f);

	float3 maxColor = 0.0f;
	float alpha = 0.0f;
	for (int i = -cb_pbrData.Blur.Radius; i <= cb_pbrData.Blur.Radius; ++i)
	{
		const float4 color = tex.Sample(splr_LR, tc + delta * i);
		alpha += color.a * cb_pbrData.Blur.Coefficients[abs(i)];
		maxColor = max(maxColor, color.rgb);
	}
	return float4(maxColor * cb_pbrData.Blur.Intensity, alpha);
}