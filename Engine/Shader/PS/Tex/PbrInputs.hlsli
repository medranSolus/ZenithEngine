#ifndef PBR_INPUTS_PS_HLSLI
#define PBR_INPUTS_PS_HLSLI
#include "Buffers.hlsli"

TEX2D(albedo, 0, 2);
TEX2D(normalMap, 1, 2);
TEX2D(metalness, 2, 2);
TEX2D(roughness, 3, 2);
#ifdef _ZE_USE_PARALLAX
TEX2D(height, 4, 2);
#endif

#endif // PBR_INPUTS_PS_HLSLI