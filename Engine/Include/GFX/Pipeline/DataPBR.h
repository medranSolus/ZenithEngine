#pragma once
#include "XeGTAO.h"

namespace ZE::GFX::Pipeline
{
	// Global constant buffer data used by RendererPBR
	struct DataPBR
	{
		static constexpr S32 BLUR_KERNEL_RADIUS = 7;

		// Should be multiple of 16 B (alignment restrictions)
		XeGTAO::GTAOConstants SsaoData;

		Float3 AmbientLight;
		float HDRExposure;

		U32 BlurWidth;
		U32 BlurHeight;
		// Must not exceed coefficients size
		S32 BlurRadius;
		float BlurIntensity;

		float SsaoSliceCount;
		float SsaoStepsPerSlice;

		float Gamma;
		float GammaInverse;

		float ShadowMapSize;
		float ShadowBias;
		float ShadowNormalOffset;

		float _Padding;

		// Should be 6 * sigma - 1, current sigma for best effect 1.3 (but with reduced render target can be 2.6)
		Float4 BlurCoefficients[BLUR_KERNEL_RADIUS + 1];
	};

	// Dynamic constant buffer data used by RendererPBR
	struct CameraPBR
	{
		Matrix View;
		Matrix ViewProjection;
		Matrix ViewProjectionInverse;
		Float3 CameraPos;
		float NearClip;
		float FarClip;
	};
}