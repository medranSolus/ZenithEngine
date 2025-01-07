#pragma once
#include "GFX/CommandList.h"
ZE_WARNING_PUSH
#include "nvsdk_ngx_vk.h"
ZE_WARNING_POP

namespace ZE::RHI::VK
{
	class NgxInterface final
	{
	public:
		NgxInterface() = default;
		ZE_CLASS_MOVE(NgxInterface);
		~NgxInterface() = default;

		NVSDK_NGX_Result InitNGX(GFX::Device& dev, const NVSDK_NGX_FeatureCommonInfo& info) noexcept;
		NVSDK_NGX_Result Shutdown(GFX::Device& dev) noexcept;

		NVSDK_NGX_Result AllocateParameter(NVSDK_NGX_Parameter*& param) const noexcept;
		NVSDK_NGX_Result GetCapabilities(NVSDK_NGX_Parameter*& param) const noexcept;
		NVSDK_NGX_Result DestroyParameter(NVSDK_NGX_Parameter* param) const noexcept;

		NVSDK_NGX_Result GetScratchBufferSize(NVSDK_NGX_Feature feature,
			const NVSDK_NGX_Parameter* param, U64& bytes) const noexcept;
		NVSDK_NGX_Result GetFeatureRequirements(GFX::Device& dev,
			const NVSDK_NGX_FeatureDiscoveryInfo& featureInfo,
			NVSDK_NGX_FeatureRequirement& requirements) const noexcept;

		NVSDK_NGX_Result CreateFeature(GFX::Device& dev, GFX::CommandList& cl, NVSDK_NGX_Feature feature,
			NVSDK_NGX_Parameter* param, NVSDK_NGX_Handle*& handle) const noexcept;
		NVSDK_NGX_Result EvaluateFeature(GFX::Device& dev, GFX::CommandList& cl, const NVSDK_NGX_Handle* handle,
			const NVSDK_NGX_Parameter* param, PFN_NVSDK_NGX_ProgressCallback progress = nullptr) const noexcept;
		NVSDK_NGX_Result ReleaseFeature(NVSDK_NGX_Handle* handle) const noexcept;
	};
}