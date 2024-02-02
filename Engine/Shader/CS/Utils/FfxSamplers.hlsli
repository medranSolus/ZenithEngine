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

#if defined(_ZE_FFX_FSR2) || defined(_ZE_FFX_FSR1)
SamplerState splr_LinearClamp : register(s0);
#endif

#ifdef _ZE_FFX_SSSR
SamplerState splr_EnvironmentMap : register(s0);
#endif

#endif // FFX_SAMPLERS_CS_HLSLI