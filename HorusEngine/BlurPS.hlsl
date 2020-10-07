#include "GaussPB.hlsli"
#include "BlurDirectionPB.hlsli"

SamplerState splr : register(s0);
Texture2D tex : register(t0);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	uint width, height;
	tex.GetDimensions(width, height);
	float dx, dy;
	if (cb_vertical)
	{
		dx = 0.0f;
		dy = 1.0f / height;
	}
	else
	{
		dx = 1.0f / width;
		dy = 0.0f;
	}
	float3 maxColor = 0.0f;
	float alpha = 0.0f;
	for (int i = -cb_radius; i <= cb_radius; ++i)
	{
		const float4 color = tex.Sample(splr, tc + float2(dx * i, dy * i));
		alpha += color.a * cb_coefficients[abs(i)];
		maxColor = max(maxColor, color.rgb);
	}
	return float4(maxColor, alpha);
}