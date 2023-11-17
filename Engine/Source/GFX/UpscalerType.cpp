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
		}
	}
}