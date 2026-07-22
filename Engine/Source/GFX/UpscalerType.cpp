#include "GFX/UpscalerType.h"
#include "GFX/External/InterfaceStorage.h"
#include "GFX/Device.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_fsr1.h"
#include "FidelityFX/host/ffx_fsr2.h"
#include "FidelityFX/host/ffx_fsr3upscaler.h"
#if _ZE_FFX_API_ENABLED
#	include "ffx_upscale.h"
#endif
ZE_WARNING_POP

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
		case UpscalerType::Fsr3:
		{
			UInt2 renderSize = {};
			const FfxFsr3UpscalerQualityMode fsr3Quality = quality == UINT32_MAX ? FFX_FSR3UPSCALER_QUALITY_MODE_NATIVEAA : static_cast<FfxFsr3UpscalerQualityMode>(quality);
			ffxFsr3UpscalerGetRenderResolutionFromQualityMode(&renderSize.X, &renderSize.Y, targetSize.X, targetSize.Y, fsr3Quality);
			return renderSize;
		}
#if _ZE_FFX_API_ENABLED
		case UpscalerType::FfxFsr:
		{
			UInt2 renderSize = {};
			auto ffx = External::InterfaceStorage::CreateConnectionFfxApi();
			if (ffx)
			{
				ffxQueryDescUpscaleGetRenderResolutionFromQualityMode queryDesc = { FFX_API_QUERY_DESC_TYPE_UPSCALE_GETRENDERRESOLUTIONFROMQUALITYMODE, nullptr };
				queryDesc.displayWidth = targetSize.X;
				queryDesc.displayHeight = targetSize.Y;
				queryDesc.qualityMode = quality == UINT32_MAX ? FFX_UPSCALE_QUALITY_MODE_NATIVEAA : static_cast<FfxApiUpscaleQualityMode>(quality);
				queryDesc.pOutRenderWidth = &renderSize.X;
				queryDesc.pOutRenderHeight = &renderSize.Y;
				ffxReturnCode_t res = ffx->GetFunctions().Query(nullptr, &queryDesc.header);
				External::InterfaceStorage::ReleaseConnectionFfxApi();

				if (res == FFX_API_RETURN_OK)
					return renderSize;
				ZE_CODE_ERROR(ZE_FFX_API_ERROR(res), "Failed obtaining render resolution from FFX API!");
			}
			return targetSize;
		}
#endif
#if _ZE_XESS_ENABLED
		case UpscalerType::XeSS:
		{
			auto xess = External::InterfaceStorage::CreateConnectionXeSS(dev);
			if (xess)
			{
				xess_2d_t renderSize = {};
				const xess_2d_t output = { targetSize.X, targetSize.Y };
				const xess_quality_settings_t xessQuality = quality == UINT32_MAX ? XESS_QUALITY_SETTING_AA : static_cast<xess_quality_settings_t>(quality);
				xess_result_t res = xessGetInputResolution(xess->GetCtx(), &output, xessQuality, &renderSize);
				External::InterfaceStorage::ReleaseConnectionXeSS();

				if (res == XESS_RESULT_SUCCESS)
					return { renderSize.x, renderSize.y };
				ZE_CODE_ERROR(ZE_XESS_ERROR(res), "Error retrieving XeSS render resolution!");
			}
			return targetSize;
		}
#endif
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
#if _ZE_DLSS_ENABLED
		case UpscalerType::DLSS:
		{
			auto ngx = External::InterfaceStorage::CreateConnectionNGX(dev);
			if (ngx)
			{
				const NVSDK_NGX_PerfQuality_Value ngxQuality = quality == UINT32_MAX ? NVSDK_NGX_PerfQuality_Value_DLAA : static_cast<NVSDK_NGX_PerfQuality_Value>(quality);
				UInt2 renderSize = ngx->GetRenderSize(targetSize, ngxQuality);
				External::InterfaceStorage::ReleaseConnectionNGX();
				return renderSize;
			}
			return targetSize;
		}
#endif
		}
	}

	void CalculateJitter(Device& dev, U32& phaseIndex, float& jitterX, float& jitterY,
		UInt2 renderSize, UInt2 displaySize, UpscalerType upscaling) noexcept
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
		// XeSS and DLSS can use same jitter as FSR2 (halton sequence)
		case UpscalerType::Fsr2:
		case UpscalerType::XeSS:
		case UpscalerType::DLSS:
		{
			U32 phaseCount = ffxFsr2GetJitterPhaseCount(renderSize.X, displaySize.X);
			phaseIndex = (phaseIndex + 1) % phaseCount;
			ffxFsr2GetJitterOffset(&jitterX, &jitterY, phaseIndex, phaseCount);
			break;
		}
		case UpscalerType::FfxFsr:
		{
#if _ZE_FFX_API_ENABLED
			auto ffx = External::InterfaceStorage::CreateConnectionFfxApi();
			if (ffx)
			{
				ffxQueryDescUpscaleGetJitterPhaseCount phaseDesc = { FFX_API_QUERY_DESC_TYPE_UPSCALE_GETJITTERPHASECOUNT, nullptr };
				ffxQueryDescUpscaleGetJitterOffset jitterDesc = { FFX_API_QUERY_DESC_TYPE_UPSCALE_GETJITTEROFFSET, nullptr };
				phaseDesc.renderWidth = renderSize.X;
				phaseDesc.displayWidth = displaySize.X;
				phaseDesc.pOutPhaseCount = &jitterDesc.phaseCount;
				ffxReturnCode_t code = ffx->GetFunctions().Query(nullptr, &phaseDesc.header);

				if (code == FFX_API_RETURN_OK)
				{
					jitterDesc.index = (phaseIndex + 1) % jitterDesc.phaseCount;
					jitterDesc.pOutX = &jitterX;
					jitterDesc.pOutY = &jitterY;
					code = ffx->GetFunctions().Query(nullptr, &jitterDesc.header);
					if (code == FFX_API_RETURN_OK)
						phaseIndex = static_cast<U32>(jitterDesc.index);
					else
					{
						ZE_CODE_ERROR(ZE_FFX_API_ERROR(code), "Failed obtaining jitter offset from FFX API!");
					}
				}
				else
				{
					ZE_CODE_ERROR(ZE_FFX_API_ERROR(code), "Failed obtaining jitter phase count from FFX API!");
				}
				External::InterfaceStorage::ReleaseConnectionFfxApi();
			}
			else
			{
				ZE_FAIL("Cannot obtain FFX API functions!");
				phaseIndex = 0;
				jitterX = 0.0f;
				jitterY = 0.0f;
			}
			break;
#else
			[[fallthrough]]; // Safe fallback to FSR3 jitter calculation (should be same anyway but who knows future(tm)..)
#endif
		}
		case UpscalerType::Fsr3:
		{
			U32 phaseCount = ffxFsr3UpscalerGetJitterPhaseCount(renderSize.X, displaySize.X);
			phaseIndex = (phaseIndex + 1) % phaseCount;
			ffxFsr3UpscalerGetJitterOffset(&jitterX, &jitterY, phaseIndex, phaseCount);
			break;
		}
		}
		// Pack jitter projection space
		jitterX = 2.0f * jitterX / Utils::SafeCast<float>(renderSize.X);
		jitterY = -2.0f * jitterY / Utils::SafeCast<float>(renderSize.Y);
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
		case UpscalerType::Fsr3:
		case UpscalerType::FfxFsr:
			return log2f(Utils::SafeCast<float>(renderWidth) / Utils::SafeCast<float>(targetWidth)) - 1.0f;
		case UpscalerType::XeSS:
			return log2f(Utils::SafeCast<float>(renderWidth) / Utils::SafeCast<float>(targetWidth));
		case UpscalerType::NIS:
			return log2f(Utils::SafeCast<float>(renderWidth) / Utils::SafeCast<float>(targetWidth)) + 1e-4f;
		case UpscalerType::DLSS:
			return log2f(Utils::SafeCast<float>(renderWidth) / Utils::SafeCast<float>(targetWidth)) - 1.0f + 1e-4f;
		}
	}

	bool IsUpscalerSupported(Device& dev, UpscalerType type) noexcept
	{
		switch (type)
		{
		case UpscalerType::None:
		case UpscalerType::Fsr1:
		case UpscalerType::Fsr2:
		case UpscalerType::Fsr3:
		case UpscalerType::NIS:
			return true;
		case UpscalerType::FfxFsr:
		{
			switch (Settings::GetGfxApi())
			{
			case GfxApiType::DX12:
				return true;
			default:
				return false;
			}
		}
		case UpscalerType::XeSS:
		{
			switch (Settings::GetGfxApi())
			{
			case GfxApiType::DX12:
			case GfxApiType::Vulkan:
				return true;
			case GfxApiType::DX11:
				return Settings::GpuVendor == VendorGPU::Intel;
			default:
				return false;
			}
		}
		case UpscalerType::DLSS:
		{
#if _ZE_DLSS_ENABLED
			if (Settings::GpuVendor == VendorGPU::Nvidia)
			{
				// Cache this variable to not query driver every time
				static struct
				{
					// 0 - not checked, 1 - available, 2 - not available
					U8 Code = 0;
					GfxApiType LastApiCheck = GfxApiType::OpenGL;
				} status = {};

				if (status.Code == 0 || status.LastApiCheck != Settings::GetGfxApi())
				{
					status.LastApiCheck = Settings::GetGfxApi();
					auto ngx = External::InterfaceStorage::CreateConnectionNGX(dev);
					if (ngx)
					{
						if (ngx->IsFeatureAvailable(dev, NVSDK_NGX_Feature_SuperSampling))
							status.Code = 1;
						External::InterfaceStorage::ReleaseConnectionNGX();
					}
					else
						status.Code = 2;
				}
				return status.Code == 1;
			}
#endif
			return false;
		}
		default:
			return false;
		}
	}
}