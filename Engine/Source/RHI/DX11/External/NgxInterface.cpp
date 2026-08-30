#include "RHI/DX11/External/NgxInterface.h"
#include "GFX/CommandList.h"

namespace ZE::RHI::DX11::External
{
	void NgxInterface::Destroy() noexcept
	{
		if (srcDev)
		{
			Status code = ZE_NGX_ERROR(NVSDK_NGX_D3D11_Shutdown1(srcDev.Get()));
			if (code)
			{
				ZE_CODE_ERROR(code, "Failed to shutdown NGX!");
			}
		}
	}

	Expected<NgxInterface> NgxInterface::Create(GFX::Device& dev, const NVSDK_NGX_FeatureCommonInfo& info) noexcept
	{
		std::wstring dataPath = Utils::ToUTF16(Logger::GetDir());
#if ZE_NGX_ID
		Status code = ZE_NGX_ERROR(NVSDK_NGX_D3D11_Init(ZE_NGX_ID, dataPath.c_str(), dev.Get().dx11.GetDevice(), &info, NVSDK_NGX_Version_API));
#else
		Status code = ZE_NGX_ERROR(NVSDK_NGX_D3D11_Init_with_ProjectID(Settings::ENGINE_UUID, NVSDK_NGX_ENGINE_TYPE_CUSTOM,
			Settings::ENGINE_VERSION_STR, dataPath.c_str(),
			dev.Get().dx11.GetDevice(), &info, NVSDK_NGX_Version_API));
#endif
		if (code)
			return std::unexpected(code);

		NgxInterface ngx;
		ngx.srcDev = dev.Get().dx11.GetDev();
		return ngx;
	}

	Status NgxInterface::AllocateParameter(NVSDK_NGX_Parameter*& param) const noexcept
	{
		return ZE_NGX_ERROR(NVSDK_NGX_D3D11_AllocateParameters(&param));
	}

	Status NgxInterface::GetCapabilities(Ptr<NVSDK_NGX_Parameter>& param) const noexcept
	{
		return ZE_NGX_ERROR(NVSDK_NGX_D3D11_GetCapabilityParameters(&param));
	}

	Status NgxInterface::DestroyParameter(NVSDK_NGX_Parameter* param) const noexcept
	{
		return ZE_NGX_ERROR(NVSDK_NGX_D3D11_DestroyParameters(param));
	}

	Status NgxInterface::GetScratchBufferSize(NVSDK_NGX_Feature feature,
		const NVSDK_NGX_Parameter* param, U64& bytes) const noexcept
	{
		return ZE_NGX_ERROR(NVSDK_NGX_D3D11_GetScratchBufferSize(feature, param, &bytes));
	}

	Status NgxInterface::GetFeatureRequirements(GFX::Device& dev,
		const NVSDK_NGX_FeatureDiscoveryInfo& featureInfo, NVSDK_NGX_FeatureRequirement& requirements) const noexcept
	{
		// Retrieve adapter from device
		DX::ComPtr<DX::IDevice> dxgiDevice;
		if (SUCCEEDED(dev.Get().dx11.GetDev().As(&dxgiDevice)))
		{
			DX::ComPtr<IDXGIAdapter> adapter;
			if (SUCCEEDED(dxgiDevice->GetAdapter(&adapter)))
				return ZE_NGX_ERROR(NVSDK_NGX_D3D11_GetFeatureRequirements(adapter.Get(), &featureInfo, &requirements));
		}
		return ZE_NGX_ERROR(NVSDK_NGX_Result_FAIL_PlatformError);
	}

	Status NgxInterface::CreateFeature(GFX::Device& dev, GFX::CommandList& cl, NVSDK_NGX_Feature feature,
		NVSDK_NGX_Parameter* param, NVSDK_NGX_Handle*& handle) const noexcept
	{
		return ZE_NGX_ERROR(NVSDK_NGX_D3D11_CreateFeature(dev.Get().dx11.GetMainContext(), feature, param, &handle));
	}

	Status NgxInterface::EvaluateFeature(GFX::Device& dev, GFX::CommandList& cl, const NVSDK_NGX_Handle* handle,
		const NVSDK_NGX_Parameter* param, PFN_NVSDK_NGX_ProgressCallback progress) const noexcept
	{
		ZE_WARNING_DISABLE_MSVC(5039); // Progress callback is noexcept
		return ZE_NGX_ERROR(NVSDK_NGX_D3D11_EvaluateFeature(cl.Get().dx11.GetContext(), handle, param, progress));
	}

	Status NgxInterface::ReleaseFeature(NVSDK_NGX_Handle* handle) const noexcept
	{
		return ZE_NGX_ERROR(NVSDK_NGX_D3D11_ReleaseFeature(handle));
	}
}