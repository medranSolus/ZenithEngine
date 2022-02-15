#pragma once

namespace ZE::GFX::Pipeline
{
	// Global constant buffer data used by RendererPBR
	struct DataPBR
	{
		static constexpr U32 SSAO_KERNEL_MAX_SIZE = 32;
		static constexpr S32 BLUR_KERNEL_RADIUS = 7;

		Matrix ViewProjection;
		Matrix ViewProjectionInverse;
		float NearClip;
		float FarClip;
		float Gamma;
		float GammaInverse;
		Float3 AmbientLight;
		float HDRExposure;
		UInt2 FrameDimmensions;

		struct
		{
			UInt2 NoiseDimmensions;
			float Bias;
			float SampleRadius;
			float Power;
			U32 KernelSize;
			Float4 Kernel[SSAO_KERNEL_MAX_SIZE];
		} SSAO;

		struct
		{
			// Must not exceed coefficients size
			S32 Radius;
			U32 Width;
			U32 Height;
			float Intensity;
			// Should be 6 * sigma - 1, current sigma for best effect 1.3 (but with reduced render target can be 2.6)
			Float4 Coefficients[BLUR_KERNEL_RADIUS + 1];
		} Blur;
	};
}