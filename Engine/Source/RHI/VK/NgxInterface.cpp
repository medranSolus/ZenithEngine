#include "RHI/VK/NgxInterface.h"

namespace ZE::RHI::VK
{
	NVSDK_NGX_Result NgxInterface::InitNGX(GFX::Device& dev, const NVSDK_NGX_FeatureCommonInfo& info) const noexcept
	{
		Device& device = dev.Get().vk;
#if ZE_NGX_ID
		return NVSDK_NGX_VULKAN_Init(ZE_NGX_ID, Logger::LOG_DIR_W,
			device.GetInstance(), device.GetPhysicalDevice(), device.GetDevice(),
			vkGetInstanceProcAddr, vkGetDeviceProcAddr, &info, NVSDK_NGX_Version_API);
#else
		return NVSDK_NGX_VULKAN_Init_with_ProjectID(Settings::ENGINE_UUID, NVSDK_NGX_ENGINE_TYPE_CUSTOM,
			Settings::ENGINE_VERSION_STR, Logger::LOG_DIR_W,
			device.GetInstance(), device.GetPhysicalDevice(), device.GetDevice(),
			vkGetInstanceProcAddr, vkGetDeviceProcAddr, &info, NVSDK_NGX_Version_API);
#endif
	}

	NVSDK_NGX_Result NgxInterface::Shutdown(GFX::Device& dev) const noexcept
	{
		return NVSDK_NGX_VULKAN_Shutdown1(dev.Get().vk.GetDevice());
	}

	NVSDK_NGX_Result NgxInterface::AllocateParameter(NVSDK_NGX_Parameter*& param) const noexcept
	{
		return NVSDK_NGX_VULKAN_AllocateParameters(&param);
	}

	NVSDK_NGX_Result NgxInterface::GetCapabilities(NVSDK_NGX_Parameter*& param) const noexcept
	{
		return NVSDK_NGX_VULKAN_GetCapabilityParameters(&param);
	}

	NVSDK_NGX_Result NgxInterface::DestroyParameter(NVSDK_NGX_Parameter* param) const noexcept
	{
		return NVSDK_NGX_VULKAN_DestroyParameters(param);
	}

	NVSDK_NGX_Result NgxInterface::GetScratchBufferSize(NVSDK_NGX_Feature feature,
		const NVSDK_NGX_Parameter* param, U64& bytes) const noexcept
	{
		return NVSDK_NGX_VULKAN_GetScratchBufferSize(feature, param, &bytes);
	}

	NVSDK_NGX_Result NgxInterface::GetFeatureRequirements(GFX::Device& dev,
		const NVSDK_NGX_FeatureDiscoveryInfo& featureInfo, NVSDK_NGX_FeatureRequirement& requirements) const noexcept
	{
		return NVSDK_NGX_VULKAN_GetFeatureRequirements(device.GetInstance(), device.GetPhysicalDevice(), &featureInfo, &requirements);
	}

	NVSDK_NGX_Result NgxInterface::CreateFeature(GFX::Device& dev, GFX::CommandList& cl, NVSDK_NGX_Feature feature,
		NVSDK_NGX_Parameter* param, NVSDK_NGX_Handle*& handle) const noexcept
	{
		return NVSDK_NGX_VULKAN_CreateFeature1(dev.Get().vk.GetDevice(), cl.Get().vk.GetBuffer(), feature, param, &handle);
	}

	NVSDK_NGX_Result NgxInterface::EvaluateFeature(GFX::Device& dev, GFX::CommandList& cl, const NVSDK_NGX_Handle* handle,
		const NVSDK_NGX_Parameter* param, PFN_NVSDK_NGX_ProgressCallback progress) const noexcept
	{
		ZE_WARNING_DISABLE_MSVC(5039); // Progress callback is noexcept
		return NVSDK_NGX_VULKAN_EvaluateFeature(cl.Get().vk.GetBuffer(), handle, param, progress);
	}

	NVSDK_NGX_Result NgxInterface::ReleaseFeature(NVSDK_NGX_Handle* handle) const noexcept
	{
		return NVSDK_NGX_VULKAN_ReleaseFeature(handle);
	}
}