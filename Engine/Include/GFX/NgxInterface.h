#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/NgxInterface.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/NgxInterface.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/NgxInterface.h"
#endif
#include "RHI/Backend.h"

namespace ZE::GFX
{
	// Helper class for accessing Nvidia NGX methods
	class NgxInterface final
	{
		enum class FeatureString : U8 { Name, ScaleFactor, Available, DriverUpdate, DriverMinVersionMajor, DriverMinVersionMinor, InitResult };

		typedef NVSDK_NGX_Result(NVSDK_CONV* PFN_NVSDK_NGX_DLSS_GetOptimalSettingsCallback)(NVSDK_NGX_Parameter* prams);

		static inline bool ignoreInternalLogs = true;

		ZE_RHI_BACKEND(NgxInterface);
		NVSDK_NGX_Parameter* ngxCaps = nullptr;
		PFN_NVSDK_NGX_DLSS_GetOptimalSettingsCallback optimalSettingsCallback = nullptr;

		static constexpr const char* GetFeatureSupportResult(NVSDK_NGX_Feature_Support_Result res) noexcept;
		static constexpr const char* GetFeatureString(NVSDK_NGX_Feature feature, FeatureString stringType) noexcept;
		static constexpr const char* GetResultString(NVSDK_NGX_Result res) noexcept;

		static void NVSDK_CONV MessageHandler(const char* message, NVSDK_NGX_Logging_Level loggingLevel, NVSDK_NGX_Feature sourceComponent) noexcept;
		static NVSDK_NGX_FeatureCommonInfo GetCommonInfo() noexcept;
		static void FreeScratchBuffer(NVSDK_NGX_Parameter* param) noexcept;

	public:
		NgxInterface() = default;
		ZE_CLASS_MOVE(NgxInterface);
		~NgxInterface() = default;

		constexpr bool IsInitialized() const noexcept { return ngxCaps && optimalSettingsCallback; }

		bool Init(Device& dev, bool ignoreInternalMsg = true) noexcept;
		void Free(Device& dev) noexcept;

		NVSDK_NGX_Parameter* AllocateParameter() const noexcept;
		NVSDK_NGX_Handle* CreateFeature(Device& dev, NVSDK_NGX_Feature feature, NVSDK_NGX_Parameter* initParam) const noexcept;
		bool RunFeature(Device& dev, CommandList& cl, const NVSDK_NGX_Handle* feature, const NVSDK_NGX_Parameter* param) const noexcept;
		// Parameters used for initialization of the features must be freed after freeing the feature itself
		void FreeParameter(NVSDK_NGX_Parameter* param) const noexcept;
		void FreeFeature(NVSDK_NGX_Handle* feature) const noexcept;

		bool IsFeatureAvailable(Device& dev, NVSDK_NGX_Feature feature) const noexcept;
		UInt2 GetRenderSize(UInt2 targetSize, NVSDK_NGX_PerfQuality_Value quality) const noexcept;
	};
}