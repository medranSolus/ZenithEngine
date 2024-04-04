#pragma once
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_fsr1.h"
#include "FidelityFX/host/ffx_fsr2.h"
#include "xess/xess.h"
ZE_WARNING_POP

namespace ZE::GFX
{
	// Type of possible upscaler in graphics pipeline
	enum class UpscalerType : U8
	{
		None,
		Fsr1,
		Fsr2,
		XeSS,
		NIS
	};

	// Configuration of possible quality modes for NIS upsacler
	enum class NISQualityMode : U8 { MegaQuality, UltraQuality, Quality, Balanced, Performance };

	// Get jitter subpixel offsets in UV space
	constexpr float GetReactiveMaskClamp(UpscalerType upscaling) noexcept;
	constexpr void CalculateJitter(U32& phaseIndex, float& jitterX, float& jitterY, UInt2 renderSize, UpscalerType upscaling) noexcept;

	// Pass UINT32_MAX for max quality regardles of upscaler
	UInt2 CalculateRenderSize(class Device& dev, UInt2 targetSize, UpscalerType upscaling, U32 quality) noexcept;
	float CalculateMipBias(U32 renderWidth, U32 targetWidth, UpscalerType upscaling) noexcept;

#pragma region Functions
	constexpr float GetReactiveMaskClamp(UpscalerType upscaling) noexcept
	{
		switch (upscaling)
		{
		case UpscalerType::Fsr2:
			return 0.9f;
		case UpscalerType::XeSS:
			return 1.0f;
		default:
			return 0.0f;
		}
	}

	constexpr void CalculateJitter(U32& phaseIndex, float& jitterX, float& jitterY, UInt2 renderSize, UpscalerType upscaling) noexcept
	{
		switch (upscaling)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case UpscalerType::None:
		case UpscalerType::Fsr1:
		case UpscalerType::NIS:
		{
			phaseIndex = 0;
			jitterX = 0.0f;
			jitterY = 0.0f;
			break;
		}
		case UpscalerType::Fsr2:
		case UpscalerType::XeSS: // XeSS can use same jitter as FSR2
		{
			U32 phaseCount = ffxFsr2GetJitterPhaseCount(renderSize.X, renderSize.X);
			phaseIndex = (phaseIndex + 1) % phaseCount;
			ffxFsr2GetJitterOffset(&jitterX, &jitterY, phaseIndex, phaseCount);
			jitterX = 2.0f * jitterX / Utils::SafeCast<float>(renderSize.X);
			jitterY = -2.0f * jitterY / Utils::SafeCast<float>(renderSize.Y);
			break;
		}
		}
	}
#pragma endregion
}