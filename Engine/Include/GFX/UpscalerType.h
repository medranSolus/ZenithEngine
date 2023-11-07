#pragma once
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_fsr2.h"
ZE_WARNING_POP

namespace ZE::GFX
{
	// Type of possible upscaler in graphics pipeline
	enum class UpscalerType : U8
	{
		None,
		Fsr2
	};

	constexpr UInt2 CalculateRenderSize(UInt2 targetSize, UpscalerType upscaling) noexcept;
	// Get jitter subpixel offsets in UV space
	constexpr void CalculateJitter(U32& phaseIndex, float& jitterX, float& jitterY, UInt2 renderSize, UpscalerType upscaling) noexcept;

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

	constexpr void CalculateJitter(U32& phaseIndex, float& jitterX, float& jitterY, UInt2 renderSize, UpscalerType upscaling) noexcept
	{
		switch (upscaling)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case UpscalerType::None:
		{
			phaseIndex = 0;
			jitterX = 0.0f;
			jitterY = 0.0f;
			break;
		}
		case UpscalerType::Fsr2:
		{
			U32 phaseCount = ffxFsr2GetJitterPhaseCount(renderSize.X, renderSize.X);
			ffxFsr2GetJitterOffset(&jitterX, &jitterY, phaseIndex, phaseCount);
			phaseIndex = (phaseIndex + 1) % phaseCount;
			break;
		}
		}
	}
#pragma endregion
}