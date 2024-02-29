#ifndef PBR_FLAGS_PS_HLSLI
#define PBR_FLAGS_PS_HLSLI

// Flags that can be set in Pbr cbuffer
static const uint ZE_PBR_USE_ALBEDO_TEX = 1;
static const uint ZE_PBR_USE_NORMAL_TEX = 2;
static const uint ZE_PBR_USE_METAL_TEX = 4;
static const uint ZE_PBR_USE_ROUGH_TEX = 8;

#endif // PBR_FLAGS_PS_HLSLI