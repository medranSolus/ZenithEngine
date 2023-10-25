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

		UInt2 DisplaySize;
		UInt2 RenderSize;

		Float3 AmbientLight;
		float HDRExposure;

		U32 BlurWidth;
		U32 BlurHeight;
		// Must not exceed coefficients size
		S32 BlurRadius;
		float BlurIntensity;

		float Gamma;
		float GammaInverse;

		float ShadowMapSize;
		float ShadowBias;

		float ShadowNormalOffset;

		float _Padding[3];

		// Should be 6 * sigma - 1, current sigma for best effect 1.3 (but with reduced render target can be 2.6)
		Float4 BlurCoefficients[BLUR_KERNEL_RADIUS + 1];
	};

	// Dynamic constant buffer data used by RendererPBR
	struct CameraPBR
	{
		Matrix View;
		Matrix ViewProjection;
		Matrix PrevViewProjection;
		Matrix ViewProjectionInverse;
		Matrix PrevViewProjectionInverse;
		Float3 CameraPos;
		float NearClip;
		float JitterX;
		float JitterY;
		float PrevJitterX;
		float PrevJitterY;
	};
#pragma pack(pop)
}