#pragma once
#if _ZE_NGX_ENABLED
#	include "GFX/External/Error.h"
#	include "RHI/DX12/DX12.h"
ZE_WARNING_PUSH
#	include "nvsdk_ngx.h"
ZE_WARNING_POP

namespace ZE::GFX
{
	class Device;
	class CommandList;
}
namespace ZE::RHI::DX12::External
{
	class NgxInterface final
	{
		DX::ComPtr<IDevice> srcDev;

		void MoveFrom(NgxInterface&& ngx) noexcept { srcDev = std::exchange(ngx.srcDev, nullptr); }

		void Destroy() noexcept;

	public:
		NgxInterface() = default;
		ZE_CLASS_NO_COPY(NgxInterface);
		NgxInterface(NgxInterface&& ngx) noexcept { MoveFrom(std::move(ngx)); }
		NgxInterface& operator=(NgxInterface&& ngx) noexcept { Destroy(); MoveFrom(std::move(ngx));  return *this; }
		~NgxInterface() { Destroy(); }

		static Expected<NgxInterface> Create(GFX::Device& dev, const NVSDK_NGX_FeatureCommonInfo& info) noexcept;

		Status AllocateParameter(NVSDK_NGX_Parameter*& param) const noexcept;
		Status GetCapabilities(Ptr<NVSDK_NGX_Parameter>& param) const noexcept;
		Status DestroyParameter(NVSDK_NGX_Parameter* param) const noexcept;

		Status GetScratchBufferSize(NVSDK_NGX_Feature feature,
			const NVSDK_NGX_Parameter* param, U64& bytes) const noexcept;
		Status GetFeatureRequirements(GFX::Device& dev, const NVSDK_NGX_FeatureDiscoveryInfo& featureInfo,
			NVSDK_NGX_FeatureRequirement& requirements) const noexcept;

		Status CreateFeature(GFX::Device& dev, GFX::CommandList& cl, NVSDK_NGX_Feature feature,
			NVSDK_NGX_Parameter* param, NVSDK_NGX_Handle*& handle) const noexcept;
		Status EvaluateFeature(GFX::Device& dev, GFX::CommandList& cl, const NVSDK_NGX_Handle* handle,
			const NVSDK_NGX_Parameter* param, PFN_NVSDK_NGX_ProgressCallback progress = nullptr) const noexcept;
		Status ReleaseFeature(NVSDK_NGX_Handle* handle) const noexcept;
	};
}
#endif