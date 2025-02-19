#pragma once
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_fsr2.h"
#include "FidelityFX/host/ffx_fsr3upscaler.h"
ZE_WARNING_POP

namespace ZE::GFX
{
	// Type of possible upscaler in graphics pipeline
	enum class UpscalerType : U8
	{
		None,
		Fsr1,
		Fsr2,
		Fsr3,
		FfxFsr,
		XeSS,
		NIS,
		DLSS
	};

	// Configuration of possible quality modes for NIS upsacler
	enum class NISQualityMode : U8 { MegaQuality, UltraQuality, Quality, Balanced, Performance };

	// Get jitter subpixel offsets in UV space
	constexpr float GetReactiveMaskClamp(UpscalerType upscaling) noexcept;
	constexpr bool IsMotionRequired(UpscalerType upscaling) noexcept;
	constexpr bool IsJitterRequired(UpscalerType upscaling) noexcept;

	// Pass UINT32_MAX for max quality regardles of upscaler
	UInt2 CalculateRenderSize(class Device& dev, UInt2 targetSize, UpscalerType upscaling, U32 quality) noexcept;
	void CalculateJitter(Device& dev, U32& phaseIndex, float& jitterX, float& jitterY,
		UInt2 renderSize, UInt2 displaySize, UpscalerType upscaling) noexcept;
	float CalculateMipBias(U32 renderWidth, U32 targetWidth, UpscalerType upscaling) noexcept;
	bool IsUpscalerSupported(Device& dev, UpscalerType type) noexcept;

#pragma region Functions
	constexpr float GetReactiveMaskClamp(UpscalerType upscaling) noexcept
	{
		switch (upscaling)
		{
		case UpscalerType::Fsr2:
		case UpscalerType::Fsr3:
		case UpscalerType::FfxFsr:
			return 0.9f;
		case UpscalerType::XeSS:
			return 1.0f;
		default:
			return 0.0f;
		}
	}

	constexpr bool IsMotionRequired(UpscalerType upscaling) noexcept
	{
		switch (upscaling)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case UpscalerType::None:
		case UpscalerType::Fsr1:
		case UpscalerType::NIS:
			return false;
		case UpscalerType::Fsr2:
		case UpscalerType::Fsr3:
		case UpscalerType::FfxFsr:
		case UpscalerType::XeSS:
		case UpscalerType::DLSS:
			return true;
		}
	}

	constexpr bool IsJitterRequired(UpscalerType upscaling) noexcept
	{
		return IsMotionRequired(upscaling);
	}
#pragma endregion
}