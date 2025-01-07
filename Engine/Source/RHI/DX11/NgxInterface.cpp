#include "RHI/DX11/NgxInterface.h"

namespace ZE::RHI::DX11
{
	NVSDK_NGX_Result NgxInterface::InitNGX(GFX::Device& dev, const NVSDK_NGX_FeatureCommonInfo& info) noexcept
	{
#if ZE_NGX_ID
		return NVSDK_NGX_D3D11_Init(ZE_NGX_ID, Logger::LOG_DIR_W, dev.Get().dx11.GetDevice(), &info, NVSDK_NGX_Version_API);
#else
		return NVSDK_NGX_D3D11_Init_with_ProjectID(Settings::ENGINE_UUID, NVSDK_NGX_ENGINE_TYPE_CUSTOM,
			Settings::ENGINE_VERSION_STR, Logger::LOG_DIR_W,
			dev.Get().dx11.GetDevice(), &info, NVSDK_NGX_Version_API);
#endif
	}

	NVSDK_NGX_Result NgxInterface::Shutdown(GFX::Device& dev) noexcept
	{
		return NVSDK_NGX_D3D11_Shutdown1(dev.Get().dx12.GetDevice());
	}

	NVSDK_NGX_Result NgxInterface::AllocateParameter(NVSDK_NGX_Parameter*& param) const noexcept
	{
		return NVSDK_NGX_D3D11_AllocateParameters(&param);
	}

	NVSDK_NGX_Result NgxInterface::GetCapabilities(NVSDK_NGX_Parameter*& param) const noexcept
	{
		return NVSDK_NGX_D3D11_GetCapabilityParameters(&param);
	}

	NVSDK_NGX_Result NgxInterface::DestroyParameter(NVSDK_NGX_Parameter* param) const noexcept
	{
		return NVSDK_NGX_D3D11_DestroyParameters(param);
	}

	NVSDK_NGX_Result NgxInterface::GetScratchBufferSize(NVSDK_NGX_Feature feature,
		const NVSDK_NGX_Parameter* param, U64& bytes) const noexcept
	{
		return NVSDK_NGX_D3D11_GetScratchBufferSize(feature, param, &bytes);
	}

	NVSDK_NGX_Result NgxInterface::GetFeatureRequirements(GFX::Device& dev,
		const NVSDK_NGX_FeatureDiscoveryInfo& featureInfo, NVSDK_NGX_FeatureRequirement& requirements) const noexcept
	{
		// Retrieve adapter from device
		DX::ComPtr<DX::IDevice> dxgiDevice = nullptr;
		if (SUCCEEDED(dev.Get().dx11.GetDev().As(&dxgiDevice)))
		{
			DX::ComPtr<DX::IAdapter> adapter = nullptr;
			if (SUCCEEDED(dxgiDevice->GetAdapter(&adapter)))
				return NVSDK_NGX_D3D11_GetFeatureRequirements(adapter.Get(), &featureInfo, &requirements);
		}
		return NVSDK_NGX_Result_FAIL_PlatformError;
	}

	NVSDK_NGX_Result NgxInterface::CreateFeature(GFX::Device& dev, GFX::CommandList& cl, NVSDK_NGX_Feature feature,
		NVSDK_NGX_Parameter* param, NVSDK_NGX_Handle*& handle) const noexcept
	{
		return NVSDK_NGX_D3D11_CreateFeature(cl.Get().dx11.GetContext(), feature, param, &handle);
	}

	NVSDK_NGX_Result NgxInterface::EvaluateFeature(GFX::Device& dev, GFX::CommandList& cl, const NVSDK_NGX_Handle* handle,
		const NVSDK_NGX_Parameter* param, PFN_NVSDK_NGX_ProgressCallback progress) const noexcept
	{
		ZE_WARNING_DISABLE_MSVC(5039); // Progress callback is noexcept
		return NVSDK_NGX_D3D11_EvaluateFeature(cl.Get().dx11.GetContext(), handle, param, progress);
	}

	NVSDK_NGX_Result NgxInterface::ReleaseFeature(NVSDK_NGX_Handle* handle) const noexcept
	{
		return NVSDK_NGX_D3D11_ReleaseFeature(handle);
	}
}