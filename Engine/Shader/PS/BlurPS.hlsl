#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"
#include "CB/BlurDirection.hlsli"

TEX2D(image, 0);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	const float2 delta = ct_vertical ? float2(0.0f, 1.0f / cb_pbrData.BlurHeight) : float2(1.0f / cb_pbrData.BlurWidth, 0.0f);
	float3 maxColor = 0.0f;
	float alpha = 0.0f;

	[unroll(BLUR_KERNEL_MAX_SIZE * 2 + 1)]
	for (int i = -cb_pbrData.BlurRadius; i <= cb_pbrData.BlurRadius; ++i)
	{
		const float4 color = tx_image.Sample(splr_LM, tc + delta * i);
		alpha += color.a * cb_pbrData.BlurCoefficients[abs(i)];
		maxColor = max(maxColor, color.rgb);
	}
	return float4(maxColor * cb_pbrData.BlurIntensity, alpha);
}