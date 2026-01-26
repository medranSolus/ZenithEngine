#include "CB/BlurDirection.hlsli"
#include "Samplers.hlsli"
#include "SettingsDataCB.hlsli"

TEX2D(image, 0, 1);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	const float2 delta = ct_blurDir.IsVertical ? float2(0.0f, 1.0f / cb_settingsData.BlurHeight) : float2(1.0f / cb_settingsData.BlurWidth, 0.0f);
	float3 maxColor = 0.0f;
	float alpha = 0.0f;

	[unroll(ZE_BLUR_KERNEL_MAX_SIZE * 2 + 1)]
	for (int i = -cb_settingsData.BlurRadius; i <= cb_settingsData.BlurRadius; ++i)
	{
		const float4 color = tx_image.Sample(splr_LM, tc + delta * i);
		alpha += color.a * cb_settingsData.BlurCoefficients[abs(i)];
		maxColor = max(maxColor, color.rgb);
	}
	// TODO: Make exposure impact maxColor again by intensity 1/exposure
	return float4(maxColor, alpha);
}