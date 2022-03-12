#pragma once

namespace ZE::GFX::Pipeline
{
	// Global constant buffer data used by RendererPBR
	struct DataPBR
	{
		static constexpr U32 SSAO_KERNEL_MAX_SIZE = 32;
		static constexpr S32 BLUR_KERNEL_RADIUS = 7;

		Float3 AmbientLight;
		float HDRExposure;
		UInt2 FrameDimmensions;

		UInt2 SsaoNoiseDimmensions;
		float SsaoBias;
		float SsaoSampleRadius;
		float SsaoPower;
		U32 SsaoKernelSize;

		// Must not exceed coefficients size
		S32 BlurRadius;
		U32 BlurWidth;
		U32 BlurHeight;
		float BlurIntensity;

		float ShadowMapSize;
		float ShadowBias;
		float ShadowNormalOffset;

		float Gamma;
		float GammaInverse;
		Float3 _Padding;

		Float4 SsaoKernel[SSAO_KERNEL_MAX_SIZE];
		// Should be 6 * sigma - 1, current sigma for best effect 1.3 (but with reduced render target can be 2.6)
		Float4 BlurCoefficients[BLUR_KERNEL_RADIUS + 1];
	};

	// Dynamic constant buffer data used by RendererPBR
	struct CameraPBR
	{
		Matrix ViewProjection;
		Matrix ViewProjectionInverse;
		Float3 CameraPos;
		float NearClip;
		float FarClip;
	};
}