#pragma once
#include "GUI/DearImGui.h"
ZE_WARNING_PUSH
#include "XeGTAO.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline
{
#pragma pack(push, 1)
	// Global constant buffer data used by RendererPBR
	struct DataPBR
	{
		static constexpr S32 BLUR_KERNEL_RADIUS = 7;

		// Should be multiple of 16 B (alignment restrictions)
		XeGTAO::GTAOConstants XeGTAOData;

		UInt2 DisplaySize;
		UInt2 RenderSize;

		Float3 AmbientLight;
		float HDRExposure;

		U32 BlurWidth;
		U32 BlurHeight;
		// Must not exceed coefficients size
		S32 BlurRadius;
		float BlurIntensity;

		float XeGTAOSliceCount;
		float XeGTAOStepsPerSlice;

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
	};
#pragma pack(pop)
}