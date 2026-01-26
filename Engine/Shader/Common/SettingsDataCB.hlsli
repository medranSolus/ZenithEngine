#ifndef SETTINGS_DATA_CB_HLSLI
#define SETTINGS_DATA_CB_HLSLI
#include "Buffers.hlsli"

#define ZE_BLUR_KERNEL_MAX_SIZE 8

struct SettingsData
{
	uint2 DisplaySize;
	uint2 RenderSize;

	float3 AmbientLight;
	float ReactiveMaskClamp;

	uint BlurWidth;
	uint BlurHeight;
	// Must not exceed coefficients size
	int BlurRadius;
	float BlurSigma;
	
	float ShadowMapSize;
	float ShadowBias;
	float ShadowNormalOffset;
	float MipBias;
	
	// Should be 6 * sigma - 1, current sigma for best effect 1.3 (but with reduced render target can be 2.6)
	float BlurCoefficients[ZE_BLUR_KERNEL_MAX_SIZE];
};
CBUFFER_GLOBAL(settingsData, SettingsData, 13, 0);

#endif // SETTINGS_DATA_CB_HLSLI