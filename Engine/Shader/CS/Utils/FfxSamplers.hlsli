#ifndef FFX_SAMPLERS_CS_HLSLI
#define FFX_SAMPLERS_CS_HLSLI

// Sampler definitions for given effect
#ifdef _ZE_FFX_CACAO
SamplerState splr_PointClamp : register(s0);
SamplerState splr_PointMirror : register(s1);
SamplerState splr_LinearClamp : register(s2);
SamplerState splr_ViewspaceDepthTap : register(s3);
SamplerState splr_RealPointClamp : register(s4);
#endif

#ifdef _ZE_FFX_FSR2
SamplerState splr_LinearClamp : register(s0);
#endif

#endif // FFX_SAMPLERS_CS_HLSLI