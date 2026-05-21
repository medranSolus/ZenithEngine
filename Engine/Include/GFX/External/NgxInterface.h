#pragma once
#if _ZE_NGX_ENABLED
#	if _ZE_RHI_DX11
#		include "RHI/DX11/External/NgxInterface.h"
#	endif
#	if _ZE_RHI_DX12
#		include "RHI/DX12/External/NgxInterface.h"
#	endif
#	if _ZE_RHI_VK
#		include "RHI/VK/External/NgxInterface.h"
#	endif
#	include "RHI/Backend.h"

namespace ZE::GFX::External
{
	// Helper class for accessing Nvidia NGX methods
	class NgxInterface final
	{
		enum class FeatureString : U8 { Name, ScaleFactor, Available, DriverUpdate, DriverMinVersionMajor, DriverMinVersionMinor, InitResult };

		typedef NVSDK_NGX_Result(NVSDK_CONV* PFN_NVSDK_NGX_DLSS_GetOptimalSettingsCallback)(NVSDK_NGX_Parameter* prams);

		static inline bool ignoreInternalLogs = true;

		ZE_RHI_BACKEND(External::NgxInterface);
		Ptr<NVSDK_NGX_Parameter> ngxCaps;
		PFN_NVSDK_NGX_DLSS_GetOptimalSettingsCallback optimalSettingsCallback = nullptr;

		static constexpr const char* GetFeatureSupportResult(NVSDK_NGX_Feature_Support_Result res) noexcept;
		static constexpr const char* GetFeatureString(NVSDK_NGX_Feature feature, FeatureString stringType) noexcept;

		static void NVSDK_CONV MessageHandler(const char* message, NVSDK_NGX_Logging_Level loggingLevel, NVSDK_NGX_Feature sourceComponent) noexcept;
		static NVSDK_NGX_FeatureCommonInfo GetCommonInfo() noexcept;
		static void FreeScratchBuffer(NVSDK_NGX_Parameter* param) noexcept;

		Status GetCapabilities(Ptr<NVSDK_NGX_Parameter>& param) const noexcept { ZE_RHI_BACKEND_CALL_RET(GetCapabilities, param); }

	public:
		NgxInterface() = default;
		ZE_CLASS_MOVE(External::NgxInterface);
		~NgxInterface();

		static Expected<NgxInterface> Create(Device& dev, bool ignoreInternalMsg = true) noexcept;
		ZE_RHI_BACKEND_GET(External::NgxInterface);

		// Main Gfx API

		constexpr bool IsInitialized() const noexcept { return ngxCaps && optimalSettingsCallback; }

		Status AllocateParameter(NVSDK_NGX_Parameter*& param) const noexcept;
		Status CreateFeature(Device& dev, NVSDK_NGX_Feature type, NVSDK_NGX_Parameter* initParam, NVSDK_NGX_Handle*& feature) const noexcept;
		Status RunFeature(Device& dev, CommandList& cl, const NVSDK_NGX_Handle* feature, const NVSDK_NGX_Parameter* param) const noexcept;
		// Parameters used for initialization of the features must be freed after freeing the feature itself
		void FreeParameter(NVSDK_NGX_Parameter* param) const noexcept;
		void FreeFeature(NVSDK_NGX_Handle* feature) const noexcept;

		bool IsFeatureAvailable(Device& dev, NVSDK_NGX_Feature feature) const noexcept;
		UInt2 GetRenderSize(UInt2 targetSize, NVSDK_NGX_PerfQuality_Value quality) noexcept;
	};
}
#endif