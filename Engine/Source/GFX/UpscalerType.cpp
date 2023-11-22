#include "GFX/UpscalerType.h"
#include "GFX/Device.h"
#include "GFX/XeSSException.h"

namespace ZE::GFX
{
	UInt2 CalculateRenderSize(Device& dev, UInt2 targetSize, UpscalerType upscaling) noexcept
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
			ffxFsr1GetRenderResolutionFromQualityMode(&renderSize.X, &renderSize.Y, targetSize.X, targetSize.Y, FFX_FSR1_QUALITY_MODE_ULTRA_QUALITY);
			return renderSize;
		}
		case UpscalerType::Fsr2:
		{
			UInt2 renderSize = {};
			ffxFsr2GetRenderResolutionFromQualityMode(&renderSize.X, &renderSize.Y, targetSize.X, targetSize.Y, FFX_FSR2_QUALITY_MODE_QUALITY);
			return renderSize;
		}
		case UpscalerType::XeSS:
		{
			ZE_XESS_ENABLE();
			const xess_2d_t output = { targetSize.X, targetSize.Y };
			xess_2d_t renderSize = {};
			ZE_XESS_CHECK(xessGetInputResolution(dev.GetXeSSCtx(), &output, XESS_QUALITY_SETTING_ULTRA_QUALITY, &renderSize), "Error retrieving XeSS render resolution!");
			return { renderSize.x, renderSize.y };
		}
		case UpscalerType::NIS:
		{
			const NISQualityMode qualityMode = NISQualityMode::UltraQuality;
			float ratio = 1.0f;
			switch (qualityMode)
			{
			default:
				ZE_ENUM_UNHANDLED();
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