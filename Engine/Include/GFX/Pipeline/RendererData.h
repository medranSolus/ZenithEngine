#pragma once
#include "Types.h"

namespace ZE::GFX::Pipeline
{
#pragma pack(push, 1)
	// Global data in CBuffer used by all core passes
	struct RendererSettingsData
	{
		static constexpr S32 BLUR_KERNEL_RADIUS = 7;

		UInt2 DisplaySize;
		UInt2 RenderSize;

		Float3 AmbientLight;
		float HDRExposure;

		U32 BlurWidth;
		U32 BlurHeight;
		// Must not exceed coefficients size
		S32 BlurRadius;
		float BlurIntensity;

		float BlurSigma;
		float ShadowMapSize;
		float ShadowBias;
		float ShadowNormalOffset;

		float MipBias;
		float Gamma;
		float GammaInverse;
		float ReactiveMaskClamp;

		// Should be 6 * sigma - 1, current sigma for best effect 1.3 (but with reduced render target can be 2.6)
		Float4 BlurCoefficients[BLUR_KERNEL_RADIUS + 1];
	};

	// Main data used by core render passes for dynamic CBuffer
	struct RendererDynamicData
	{
		Float4x4 ViewTps;
		Float4x4 ViewProjectionTps;
		Float4x4 ViewProjectionInverseTps;
		Float3 CameraPos;
		float NearClip;
		Float2 JitterCurrent;
		Float2 JitterPrev;
	};
#pragma pack(pop)

	// Generic data used by render graph for all passes
	struct RendererGraphData
	{
		EID CurrentCamera;
		Float4x4 Projection;
		Float4x4 PrevViewProjectionTps;
		U32 JitterIndex = 0;
		bool FrameTemporalReset = false;
		bool UseIBL = false;
	};
}