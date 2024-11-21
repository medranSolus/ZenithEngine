#include "GFX/UpscalerType.h"
#include "GFX/Device.h"
#include "GFX/XeSSException.h"

namespace ZE::GFX
{
	UInt2 CalculateRenderSize(Device& dev, UInt2 targetSize, UpscalerType upscaling, U32 quality) noexcept
	{
		switch (upscaling)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case UpscalerType::None:
			return targetSize;
		case UpscalerType::Fsr1:
		{
			UInt2 renderSize = {};
			const FfxFsr1QualityMode fsr1Quality = quality == UINT32_MAX ? FFX_FSR1_QUALITY_MODE_ULTRA_QUALITY : static_cast<FfxFsr1QualityMode>(quality);
			ffxFsr1GetRenderResolutionFromQualityMode(&renderSize.X, &renderSize.Y, targetSize.X, targetSize.Y, fsr1Quality);
			return renderSize;
		}
		case UpscalerType::Fsr2:
		{
			UInt2 renderSize = {};
			const FfxFsr2QualityMode fsr2Quality = quality == UINT32_MAX ? FFX_FSR2_QUALITY_MODE_QUALITY : static_cast<FfxFsr2QualityMode>(quality);
			ffxFsr2GetRenderResolutionFromQualityMode(&renderSize.X, &renderSize.Y, targetSize.X, targetSize.Y, fsr2Quality);
			return renderSize;
		}
		case UpscalerType::XeSS:
		{
			ZE_XESS_ENABLE();
			xess_2d_t renderSize = {};
			const xess_2d_t output = { targetSize.X, targetSize.Y };
			const xess_quality_settings_t xessQuality = quality == UINT32_MAX ? XESS_QUALITY_SETTING_AA : static_cast<xess_quality_settings_t>(quality);
			ZE_XESS_CHECK(xessGetInputResolution(dev.GetXeSSCtx(), &output, xessQuality, &renderSize), "Error retrieving XeSS render resolution!");
			return { renderSize.x, renderSize.y };
		}
		case UpscalerType::NIS:
		{
			float ratio = 1.0f;
			const NISQualityMode nisQuality = quality == UINT32_MAX ? NISQualityMode::MegaQuality : static_cast<NISQualityMode>(quality);
			switch (nisQuality)
			{
			default:
				ZE_ENUM_UNHANDLED();
			case NISQualityMode::MegaQuality:
				ratio = 1.176f;
				break;
			case NISQualityMode::UltraQuality:
				ratio = 1.3f;
				break;
			case NISQualityMode::Quality:
				ratio = 1.5f;
				break;
			case NISQualityMode::Balanced:
				ratio = 1.7f;
				break;
			case NISQualityMode::Performance:
				ratio = 2.0f;
				break;
			}
			return
			{
				Utils::SafeCast<U32>(Utils::SafeCast<float>(targetSize.X) / ratio),
				Utils::SafeCast<U32>(Utils::SafeCast<float>(targetSize.Y) / ratio)
			};
		}
		}
	}

	float CalculateMipBias(U32 renderWidth, U32 targetWidth, UpscalerType upscaling) noexcept
	{
		switch (upscaling)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case UpscalerType::None:
		case UpscalerType::Fsr1:
			return 0.0f;
		case UpscalerType::Fsr2:
			return log2f(Utils::SafeCast<float>(renderWidth) / Utils::SafeCast<float>(targetWidth)) - 1.0f;
		case UpscalerType::XeSS:
			return log2f(Utils::SafeCast<float>(renderWidth) / Utils::SafeCast<float>(targetWidth));
		case UpscalerType::NIS:
			return log2f(Utils::SafeCast<float>(renderWidth) / Utils::SafeCast<float>(targetWidth)) + 1e-4f;
		}
	}
}