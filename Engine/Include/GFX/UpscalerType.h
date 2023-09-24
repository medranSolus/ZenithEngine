#pragma once
#include "FidelityFX/host/ffx_fsr2.h"

namespace ZE::GFX
{
	// Type of possible upscaler in graphics pipeline
	enum class UpscalerType : U8
	{
		None,
		Fsr2
	};

	constexpr UInt2 CalculateRenderSize(UInt2 targetSize, UpscalerType upscaling) noexcept;

#pragma region Functions
	constexpr UInt2 CalculateRenderSize(UInt2 targetSize, UpscalerType upscaling) noexcept
	{
		switch (upscaling)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case UpscalerType::None:
			return targetSize;
		case UpscalerType::Fsr2:
		{
			UInt2 renderSize = {};
			ffxFsr2GetRenderResolutionFromQualityMode(&renderSize.X, &renderSize.Y, targetSize.X, targetSize.Y, FFX_FSR2_QUALITY_MODE_QUALITY);
			return renderSize;
		}
		}
	}
#pragma endregion
}