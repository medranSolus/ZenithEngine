#pragma once
#include "GfxResources.h"

namespace GFX::Pipeline
{
	class Blur
	{
		static constexpr int maxRadius = 15;
		struct GaussBuffer
		{
			int radius;
			float padding[3];
			DirectX::XMFLOAT4 coefficients[maxRadius];
		};

		struct DirectionBuffer
		{
			BOOL vertical;
			float padding[3];
		};

		int radius;
		float sigma;
		Resource::PixelShader shader;
		Resource::ConstBufferPixel<GaussBuffer> kernelBuffer;
		Resource::ConstBufferPixel<DirectionBuffer> controlBuffer;

	public:
		inline Blur(Graphics& gfx, int radius = 7, float sigma = 2.6f)
			: radius(radius), sigma(sigma), shader(gfx, "BlurPS.cso"), kernelBuffer(gfx, "$blurKernel"), controlBuffer(gfx, "$blurControl", 1U) { SetKernel(gfx, radius, sigma); }

		inline void SetVertical(Graphics& gfx) { controlBuffer.Update(gfx, { TRUE }); }
		inline void SetHorizontal(Graphics& gfx) { controlBuffer.Update(gfx, { FALSE }); }

		void SetKernel(Graphics& gfx, int radius = 7, float sigma = 2.6f);
		void Bind(Graphics& gfx) noexcept;
		void ShowWindow(Graphics& gfx) noexcept;
	};
}