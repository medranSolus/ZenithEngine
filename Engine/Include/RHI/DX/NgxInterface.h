#pragma once
#include "DXGI.h"
ZE_WARNING_PUSH
#include "nvsdk_ngx_helpers.h"
ZE_WARNING_POP

namespace ZE::RHI::DX
{
	// Make common interface that is agnostic in terms of API callbacks to have common logic for accessing params and features.
	// Implement in similar manner to classic RHI proxies but put there only the code specific to said interface.
	// This way all logic will be handled by single class.
	// But what about accessing it from the pipeline level passes? Data should be available to it on every part of code,
	// setup, execute and clean, same for FfxInterface. Now only device is passed into these functions, rework of that needed.
	// Better to focus on new render graph system with better flexibility and then introducing DLSS

	// Helper class for accessing Nvida NGX methods for D3D APIs
	template<bool IS_DX12>
	class NgxInterface
	{
		enum class FeatureString : U8 { Name, Available, InitResult, DriverUpdate, DriverMinVersionMajor, DriverMinVersionMinor };

		static inline bool ignoreInternalLogs = true;

		bool initialized = false;

		static constexpr const char* GetFeatureString(NVSDK_NGX_Feature feature, FeatureString stringType) noexcept;
		static constexpr const char* GetResultString(NVSDK_NGX_Result res) noexcept;

		static void NVSDK_CONV MessageHandler(const char* message, NVSDK_NGX_Logging_Level loggingLevel, NVSDK_NGX_Feature sourceComponent) noexcept;
		static NVSDK_NGX_FeatureCommonInfo GetCommonInfo() noexcept;
		static NVSDK_NGX_Result Initialize(void* d3dDevice) noexcept;
		static void Shutdown(void* d3dDevice) noexcept;

	public:
		NgxInterface() = default;
		ZE_CLASS_DEFAULT(NgxInterface);
		~NgxInterface() { ZE_ASSERT(!initialized, "NGX library not freed before releasing!"); }

		bool IsFeatureAvailable(IAdapter* adapter, void* d3dDevice, NVSDK_NGX_Feature feature) const noexcept;
		UInt2 GetOptimalRenderSize(UInt2 targetSize, NVSDK_NGX_PerfQuality_Value quality) const noexcept;
		bool InitFeatureDLSS(void* d3dCommandList, UInt2 renderSize, UInt2 targetSize, NVSDK_NGX_PerfQuality_Value quality, NVSDK_NGX_DLSS_Feature_Flags flags, NVSDK_NGX_Handle*& featureHandle, NVSDK_NGX_Parameter*& dlssParams) const noexcept;
		void ReleaseFeature(NVSDK_NGX_Handle*& featureHandle, NVSDK_NGX_Parameter*& ngxParams) const noexcept;

		bool Init(void* d3dDevice, bool ignoreInternalMsg = true) noexcept;
		void Free(void* d3dDevice) noexcept;
	};

#pragma region Functions
	template<bool IS_DX12>
	constexpr const char* NgxInterface<IS_DX12>::GetFeatureString(NVSDK_NGX_Feature feature, FeatureString stringType) noexcept
	{
#define GET_STRING(featureName) \
	case NVSDK_NGX_Feature_##featureName: \
	{ \
		switch (stringType) \
		{ \
		case FeatureString::Name: return #featureName; \
		case FeatureString::Available: return NVSDK_NGX_Parameter_##featureName##_Available; \
		case FeatureString::InitResult: return NVSDK_NGX_Parameter_##featureName##_FeatureInitResult; \
		case FeatureString::DriverUpdate: return NVSDK_NGX_Parameter_##featureName##_NeedsUpdatedDriver; \
		case FeatureString::DriverMinVersionMajor: return NVSDK_NGX_Parameter_##featureName##_MinDriverVersionMajor; \
		case FeatureString::DriverMinVersionMinor: return NVSDK_NGX_Parameter_S##featureName##_MinDriverVersionMinor; \
		default: return ""; \
		} \
		break; \
	}
		switch (feature)
		{
			GET_STRING(SuperSampling)
				GET_STRING(InPainting)
				GET_STRING(ImageSuperResolution)
				GET_STRING(SlowMotion)
				GET_STRING(VideoSuperResolution)
				GET_STRING(ImageSignalProcessing)
				GET_STRING(DeepResolve)
				GET_STRING(FrameGeneration)
				GET_STRING(DeepDVC)
		default:
			ZE_ENUM_UNHANDLED();
		case NVSDK_NGX_Feature_Reserved0:
		case NVSDK_NGX_Feature_Reserved1:
		case NVSDK_NGX_Feature_Reserved2:
		case NVSDK_NGX_Feature_Reserved3:
		case NVSDK_NGX_Feature_Reserved13:
		case NVSDK_NGX_Feature_Count:
		case NVSDK_NGX_Feature_Reserved_SDK:
		case NVSDK_NGX_Feature_Reserved_Core:
		case NVSDK_NGX_Feature_Reserved_Unknown:
			return "";
		}
#undef GET_STRING
	}

	template<bool IS_DX12>
	constexpr const char* NgxInterface<IS_DX12>::GetResultString(NVSDK_NGX_Result res) noexcept
	{
#define GET_STRING(feature) case NVSDK_NGX_Result_##feature: return #feature
		switch (res)
		{
			GET_STRING(Success);
			GET_STRING(Fail);
			GET_STRING(PlatformError);
			GET_STRING(FeatureAlreadyExists);
			GET_STRING(FeatureNotFound);
			GET_STRING(InvalidParameter);
			GET_STRING(ScratchBufferTooSmall);
			GET_STRING(NotInitialized);
			GET_STRING(UnsupportedInputFormat);
			GET_STRING(RWFlagMissing);
			GET_STRING(MissingInput);
			GET_STRING(UnableToInitializeFeature);
			GET_STRING(OutOfDate);
			GET_STRING(OutOfGPUMemory);
			GET_STRING(UnsupportedFormat);
			GET_STRING(UnableToWriteToAppDataPath);
			GET_STRING(UnsupportedParameter);
			GET_STRING(Denied);
			GET_STRING(NotImplemented);
		default: return "UNKNOWN";
		}
#undef GET_STRING
	}

	template<bool IS_DX12>
	void NVSDK_CONV NgxInterface<IS_DX12>::MessageHandler(const char* message, NVSDK_NGX_Logging_Level loggingLevel, NVSDK_NGX_Feature sourceComponent) noexcept
	{
		if (loggingLevel == NVSDK_NGX_LOGGING_LEVEL_OFF || (ignoreInternalLogs && (sourceComponent == NVSDK_NGX_Feature_Reserved_SDK || sourceComponent == NVSDK_NGX_Feature_Reserved_Core)))
			return;
		std::string feature;
		switch (sourceComponent)
		{
		case NVSDK_NGX_Feature_Reserved0:
			feature = "[Reserved 0] ";
			break;
		case NVSDK_NGX_Feature_SuperSampling:
			feature = "[DLSS] ";
			break;
		case NVSDK_NGX_Feature_InPainting:
			feature = "[InPainting] ";
			break;
		case NVSDK_NGX_Feature_ImageSuperResolution:
			feature = "[Image Super Resolution] ";
			break;
		case NVSDK_NGX_Feature_SlowMotion:
			feature = "[Slow Motion] ";
			break;
		case NVSDK_NGX_Feature_VideoSuperResolution:
			feature = "[Video Super Resolution] ";
			break;
		case NVSDK_NGX_Feature_Reserved1:
			feature = "[Reserved 1] ";
			break;
		case NVSDK_NGX_Feature_Reserved2:
			feature = "[Reserved 2] ";
			break;
		case NVSDK_NGX_Feature_Reserved3:
			feature = "[Reserved 3] ";
			break;
		case NVSDK_NGX_Feature_ImageSignalProcessing:
			feature = "[Image Signal Processing] ";
			break;
		case NVSDK_NGX_Feature_DeepResolve:
			feature = "[Deep Resolve] ";
			break;
		case NVSDK_NGX_Feature_FrameGeneration:
			feature = "[Frame Generation] ";
			break;
		case NVSDK_NGX_Feature_DeepDVC:
			feature = "[Deep DVC] ";
			break;
		case NVSDK_NGX_Feature_Reserved13:
			feature = "[Reserved 13] ";
			break;
		default:
			ZE_ENUM_UNHANDLED();
		case NVSDK_NGX_Feature_Reserved_Unknown:
		case NVSDK_NGX_Feature_Count:
			feature = "[UNKNOWN FEATURE] ";
			break;
		case NVSDK_NGX_Feature_Reserved_SDK:
			feature = "[SDK internal] ";
			break;
		case NVSDK_NGX_Feature_Reserved_Core:
			feature = "[Core internal] ";
			break;
		}

		Logger::Info((loggingLevel == NVSDK_NGX_LOGGING_LEVEL_VERBOSE ? "[NGX VERBOSE] " : "[NGX] ") + feature + message, false, false);
	}

	template<bool IS_DX12>
	NVSDK_NGX_FeatureCommonInfo NgxInterface<IS_DX12>::GetCommonInfo() noexcept
	{
		NVSDK_NGX_FeatureCommonInfo commonInfo = {};
		commonInfo.PathListInfo.Path = nullptr;
		commonInfo.PathListInfo.Length = 0;
		commonInfo.InternalData = nullptr;
		commonInfo.LoggingInfo.LoggingCallback = MessageHandler;
		commonInfo.LoggingInfo.MinimumLoggingLevel = _ZE_MODE_PROFILE || _ZE_MODE_RELEASE ? NVSDK_NGX_LOGGING_LEVEL_OFF : (_ZE_MODE_DEV ? NVSDK_NGX_LOGGING_LEVEL_ON : NVSDK_NGX_LOGGING_LEVEL_VERBOSE);
		commonInfo.LoggingInfo.DisableOtherLoggingSinks = true;
		return commonInfo;
	}

	template<bool IS_DX12>
	NVSDK_NGX_Result NgxInterface<IS_DX12>::Initialize(void* d3dDevice) noexcept
	{
		NVSDK_NGX_Result res;

		NVSDK_NGX_FeatureCommonInfo info = GetCommonInfo();
		if constexpr (IS_DX12)
		{
#if ZE_NGX_ID
			res = NVSDK_NGX_D3D12_Init(ZE_NGX_ID, Logger::LOG_DIR_W, d3dDevice, &info, NVSDK_NGX_Version_API);
#else
			res = NVSDK_NGX_D3D12_Init_with_ProjectID(Settings::ENGINE_UUID, NVSDK_NGX_ENGINE_TYPE_CUSTOM, Settings::ENGINE_VERSION_STR, Logger::LOG_DIR_W, d3dDevice, &info, NVSDK_NGX_Version_API);
#endif
		}
		else
		{
#if ZE_NGX_ID
			res = NVSDK_NGX_D3D11_Init(ZE_NGX_ID, Logger::LOG_DIR_W, d3dDevice, &info, NVSDK_NGX_Version_API);
#else
			res = NVSDK_NGX_D3D11_Init_with_ProjectID(Settings::ENGINE_UUID, NVSDK_NGX_ENGINE_TYPE_CUSTOM, Settings::ENGINE_VERSION_STR, Logger::LOG_DIR_W, d3dDevice, &info, NVSDK_NGX_Version_API);
#endif
		}
		return res;
	}

	template<bool IS_DX12>
	void NgxInterface<IS_DX12>::Shutdown(void* d3dDevice) noexcept
	{
		NVSDK_NGX_Result res;
		if constexpr (IS_DX12)
			res = NVSDK_NGX_D3D12_Shutdown1(d3dDevice);
		else
			res = NVSDK_NGX_D3D11_Shutdown1(d3dDevice);

		ZE_ASSERT(!NVSDK_NGX_FAILED(res), "Failed to free NGX library!");
	}

	template<bool IS_DX12>
	bool NgxInterface<IS_DX12>::IsFeatureAvailable(IAdapter* adapter, void* d3dDevice, NVSDK_NGX_Feature feature) const noexcept
	{
		NVSDK_NGX_FeatureCommonInfo commonInfo = GetCommonInfo();
		NVSDK_NGX_FeatureDiscoveryInfo info = {};
		info.SDKVersion = NVSDK_NGX_Version_API;
		info.ApplicationDataPath = Logger::LOG_DIR_W;
		info.FeatureID = feature;
		info.FeatureInfo = &commonInfo;
#if ZE_NGX_ID
		info.Identifier.IdentifierType = NVSDK_NGX_Application_Identifier_Type_Application_Id;
		info.Identifier.v.ApplicationId = ZE_NGX_ID;
#else
		info.Identifier.IdentifierType = NVSDK_NGX_Application_Identifier_Type_Project_Id;
		info.Identifier.v.ProjectDesc.ProjectId = Settings::ENGINE_UUID;
		info.Identifier.v.ProjectDesc.EngineType = NVSDK_NGX_ENGINE_TYPE_CUSTOM;
		info.Identifier.v.ProjectDesc.EngineVersion = Settings::ENGINE_VERSION_STR;
#endif

		// First general check if feature is supported at all
		NVSDK_NGX_FeatureRequirement supported = {};
		NVSDK_NGX_Result res;
		if constexpr (IS_DX12)
			res = NVSDK_NGX_D3D12_GetFeatureRequirements(adapter, &info, &supported);
		else
			res = NVSDK_NGX_D3D11_GetFeatureRequirements(adapter, &info, &supported);

		if (NVSDK_NGX_FAILED(res))
			return false;

		// Initialize temporary NGX if not done already
		if (!initialized && NVSDK_NGX_FAILED(Initialize(d3dDevice))
			return false;

			NVSDK_NGX_Parameter* ngxParams = nullptr;
			auto checkError = [&](NVSDK_NGX_Result res) -> bool
			{
				if (NVSDK_NGX_FAILED(res))
				{
					if (ngxParams)
					{
						NVSDK_NGX_Result initResult = NVSDK_NGX_Result_Fail;
						if (ngxParams->Get(GetFeatureString(feature, FeatureString::InitResult), reinterpret_cast<int*>(&initResult)) == NVSDK_NGX_Result_Success)
							MessageHandler((std::string(" Feature not available, result: ") + GetResultString(initResult)).c_str(), NVSDK_NGX_LOGGING_LEVEL_ON, feature);

						if constexpr (IS_DX12)
							NVSDK_NGX_D3D12_DestroyParameters(ngxParams);
						else
							NVSDK_NGX_D3D11_DestroyParameters(ngxParams);
					}
					if (!initialized)
						Shutdown(d3dDevice);
					return true;
				}
				return false;
			};

		// Final check if driver is correct and supported
		if constexpr (IS_DX12)
			res = NVSDK_NGX_D3D12_GetCapabilityParameters(&ngxParams);
		else
			res = NVSDK_NGX_D3D11_GetCapabilityParameters(&ngxParams);
		if (checkError(res))
			return false;

		S32 needsUpdatedDriver = 0;
		if (NVSDK_NGX_FAILED(ngxParams->Get(GetFeatureString(feature, FeatureString::DriverUpdate), &needsUpdatedDriver)))
			Logger::Warning("[NGX] [" + std::string(GetFeatureString(feature, FeatureString::Name)) + "] Cannot check if driver need updating!");
		else
		{
			if (needsUpdatedDriver)
			{
				U32 minDriverVersionMajor = 0;
				U32 minDriverVersionMinor = 0;
				res = ngxParams->Get(GetFeatureString(feature, FeatureString::DriverMinVersionMajor), &minDriverVersionMajor);
				if (NVSDK_NGX_FAILED(res) || NVSDK_NGX_FAILED(ngxParams->Get(GetFeatureString(feature, FeatureString::DriverMinVersionMinor), &minDriverVersionMinor)))
					Logger::Error("[NGX] [" + std::string(GetFeatureString(feature, FeatureString::Name)) + "] Feature not available due to outdated driver! Minimum supported driver: " + std::to_string(minDriverVersionMajor) + "." + std::to_string(minDriverVersionMinor));
				else
					Logger::Error("[NGX] [" + std::string(GetFeatureString(feature, FeatureString::Name)) + "] Feature not available due to outdated driver!");

				if (!initialized)
					Shutdown(d3dDevice);
				return false;
			}
		}

		S32 featureAvailable = 0;
		if (checkError(dlssParams->Get(GetFeatureString(feature, FeatureString::Available), &featureAvailable)) || !featureAvailable)
			return false;

		// Feature supported, free setup
		if constexpr (IS_DX12)
			NVSDK_NGX_D3D12_DestroyParameters(ngxParams);
		else
			NVSDK_NGX_D3D11_DestroyParameters(ngxParams);
		if (!initialized)
			Shutdown(d3dDevice);
		return true;
	}

	template<bool IS_DX12>
	UInt2 NgxInterface<IS_DX12>::GetOptimalRenderSize(UInt2 targetSize, NVSDK_NGX_PerfQuality_Value quality) const noexcept
	{
		ZE_ASSERT(initialized, "NGX library not initialized!");

		NVSDK_NGX_Parameter* ngxParams = nullptr;
		NVSDK_NGX_Result res;
		if constexpr (IS_DX12)
			res = NVSDK_NGX_D3D12_GetCapabilityParameters(&ngxParams);
		else
			res = NVSDK_NGX_D3D11_GetCapabilityParameters(&ngxParams);
		if (NVSDK_NGX_FAILED(res))
		{
			ZE_FAIL("Error getting NGX parameters!");
			return targetSize;
		}

		PFN_NVSDK_NGX_DLSS_GetOptimalSettingsCallback optimalSettingsCallback = nullptr;
		ngxParams->Get(NVSDK_NGX_Parameter_DLSSOptimalSettingsCallback, reinterpret_cast<void**>(&optimalSettingsCallback));
		if (!optimalSettingsCallback)
		{
			ZE_FAIL("Cannot access DLSS callback for optimal settings!");
			if constexpr (IS_DX12)
				NVSDK_NGX_D3D12_DestroyParameters(&ngxParams);
			else
				NVSDK_NGX_D3D11_DestroyParameters(&ngxParams);
			return targetSize;
		}

		ZE_ASSERT(quality != NVSDK_NGX_PerfQuality_Value_UltraQuality, "DLSS ultra quality not supported yet!");
		// Query for optimal sizes
		ngxParams->Set(NVSDK_NGX_Parameter_Width, targetSize.X);
		ngxParams->Set(NVSDK_NGX_Parameter_Height, targetSize.Y);
		ngxParams->Set(NVSDK_NGX_Parameter_PerfQualityValue, quality);
		ngxParams->Set(NVSDK_NGX_Parameter_RTXValue, false); // Some older DLSS dlls still expect this value to be set

		if (NVSDK_NGX_FAILED(optimalSettingsCallback(ngxParams)))
		{
			ZE_FAIL("Error retrieving optimal render sizes for DLSS!");
			if constexpr (IS_DX12)
				NVSDK_NGX_D3D12_DestroyParameters(&ngxParams);
			else
				NVSDK_NGX_D3D11_DestroyParameters(&ngxParams);
			return targetSize;
		}

		UInt2 renderSize = targetSize;
		ngxParams->Get(NVSDK_NGX_Parameter_OutWidth, &renderSize.X);
		ngxParams->Get(NVSDK_NGX_Parameter_OutHeight, &renderSize.Y);

		if constexpr (IS_DX12)
			NVSDK_NGX_D3D12_DestroyParameters(&ngxParams);
		else
			NVSDK_NGX_D3D11_DestroyParameters(&ngxParams);
		return renderSize;
	}

	template<bool IS_DX12>
	bool NgxInterface<IS_DX12>::InitFeatureDLSS(void* d3dCommandList, UInt2 renderSize, UInt2 targetSize, NVSDK_NGX_PerfQuality_Value quality, NVSDK_NGX_DLSS_Feature_Flags flags, NVSDK_NGX_Handle*& featureHandle, NVSDK_NGX_Parameter*& dlssParams) const noexcept
	{
		ZE_ASSERT(initialized, "NGX library not initialized!");

		NVSDK_NGX_Result res;
		if constexpr (IS_DX12)
			res = NVSDK_NGX_D3D12_GetCapabilityParameters(&dlssParams);
		else
			res = NVSDK_NGX_D3D11_GetCapabilityParameters(&dlssParams);
		if (NVSDK_NGX_FAILED(res))
		{
			ZE_FAIL("Error getting DLSS parameters!");
			return false;
		}

		U64 scratchBufferSize = 0;
		if constexpr (IS_DX12)
			res = NVSDK_NGX_D3D12_GetScratchBufferSize(NVSDK_NGX_Feature_SuperSampling, dlssParams, &scratchBufferSize);
		else
			res = NVSDK_NGX_D3D11_GetScratchBufferSize(NVSDK_NGX_Feature_SuperSampling, dlssParams, &scratchBufferSize);
		if (NVSDK_NGX_FAILED(res))
		{
			ZE_FAIL("Error getting DLSS scratch buffer size!");
			if constexpr (IS_DX12)
				NVSDK_NGX_D3D12_DestroyParameters(&dlssParams);
			else
				NVSDK_NGX_D3D11_DestroyParameters(&dlssParams);
			return false;
		}
		if (scratchBufferSize)
		{
			dlssParams->Set(NVSDK_NGX_Parameter_Scratch, new U8[scratchBufferSize]);
			dlssParams->Set(NVSDK_NGX_Parameter_Scratch_SizeInBytes, scratchBufferSize);
		}

		dlssParams->Set(NVSDK_NGX_Parameter_PerfQualityValue, quality);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, flags);

		dlssParams->Set(NVSDK_NGX_Parameter_Width, renderSize.X);
		dlssParams->Set(NVSDK_NGX_Parameter_Height, renderSize.Y);
		dlssParams->Set(NVSDK_NGX_Parameter_OutWidth, targetSize.X);
		dlssParams->Set(NVSDK_NGX_Parameter_OutHeight, targetSize.Y);

		dlssParams->Set(NVSDK_NGX_Parameter_RTXValue, false); // Some older DLSS dlls still expect this value to be set
		dlssParams->Set(NVSDK_NGX_Parameter_CreationNodeMask, 1U);
		dlssParams->Set(NVSDK_NGX_Parameter_VisibilityNodeMask, 1U);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Enable_Output_Subrects, 0U);

		/* Optimization presets
			NVSDK_NGX_DLSS_Hint_Render_Preset_Default -> stick to whatever Nvidia will choose over OTA
			NVSDK_NGX_DLSS_Hint_Render_Preset_A -> intended for Performance/Balanced/Quality, older one, better fighting of ghosting with missing inputs (ex. motion vectors)
			NVSDK_NGX_DLSS_Hint_Render_Preset_B -> intended for Ultra Performance, similar to A
			NVSDK_NGX_DLSS_Hint_Render_Preset_C -> intended for Performance/Balanced/Quality, favors current frame info, good for fast pacing games
			NVSDK_NGX_DLSS_Hint_Render_Preset_D -> intended for Performance/Balanced/Quality, default for this modes, favors image stability
			NVSDK_NGX_DLSS_Hint_Render_Preset_E -> unused
			NVSDK_NGX_DLSS_Hint_Render_Preset_F -> intended for Ultra Performance/DLAA, default for this modes
			NVSDK_NGX_DLSS_Hint_Render_Preset_G -> unused
		*/
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_DLAA, NVSDK_NGX_DLSS_Hint_Render_Preset_F);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraQuality, NVSDK_NGX_DLSS_Hint_Render_Preset_Default);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Quality, NVSDK_NGX_DLSS_Hint_Render_Preset_D);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Balanced, NVSDK_NGX_DLSS_Hint_Render_Preset_D);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Performance, NVSDK_NGX_DLSS_Hint_Render_Preset_D);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraPerformance, NVSDK_NGX_DLSS_Hint_Render_Preset_F);

		// Some default parameters
		dlssParams->Set(NVSDK_NGX_Parameter_DLSSMode, NVSDK_NGX_DLSS_Mode_DLSS);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Pre_Exposure, 1.0f);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Exposure_Scale, 1.0f);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Input_Color_Subrect_Base_X, 0U);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Input_Color_Subrect_Base_Y, 0U);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Input_Depth_Subrect_Base_X, 0U);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Input_Depth_Subrect_Base_Y, 0U);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Input_MV_SubrectBase_X, 0U);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Input_MV_SubrectBase_Y, 0U);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Input_Translucency_SubrectBase_X, 0U);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Input_Translucency_SubrectBase_Y, 0U);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_SubrectBase_X, 0U);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_SubrectBase_Y, 0U);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Output_Subrect_Base_X, 0U);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Output_Subrect_Base_Y, 0U);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Indicator_Invert_X_Axis, 0U);
		dlssParams->Set(NVSDK_NGX_Parameter_DLSS_Indicator_Invert_Y_Axis, 0U);
		dlssParams->Set(NVSDK_NGX_Parameter_Sharpness, 0.0f); // DLSS preffers separate sharpening pass later on

		// Manual resource creation
		//dlssParams->Set(NVSDK_NGX_Parameter_ResourceAllocCallback, AllocNGXResourceD3D12);
		//dlssParams->Set(NVSDK_NGX_Parameter_BufferAllocCallback, AllocNGXBufferD3D11);
		//dlssParams->Set(NVSDK_NGX_Parameter_Tex2DAllocCallback, AllocNGXTexture2DD3D11);
		//dlssParams->Set(NVSDK_NGX_Parameter_ResourceReleaseCallback, ReleaseNGXResourceD3D);

		NVSDK_NGX_Handle* featureHandle = nullptr;
		if constexpr (IS_DX12)
			res = NVSDK_NGX_D3D12_CreateFeature(d3dCommandList, NVSDK_NGX_Feature_SuperSampling, dlssParams, &featureHandle);
		else
			res = NVSDK_NGX_D3D11_CreateFeature(d3dCommandList, NVSDK_NGX_Feature_SuperSampling, dlssParams, &featureHandle);
		if (NVSDK_NGX_FAILED(res))
		{
			ZE_FAIL("Error initializing DLSS!");
			if constexpr (IS_DX12)
				NVSDK_NGX_D3D12_DestroyParameters(&dlssParams);
			else
				NVSDK_NGX_D3D11_DestroyParameters(&dlssParams);
			return false;
		}
		return true;
	}

	template<bool IS_DX12>
	void NgxInterface<IS_DX12>::ReleaseFeature(NVSDK_NGX_Handle*& featureHandle, NVSDK_NGX_Parameter*& ngxParams) const noexcept
	{
		ZE_ASSERT(initialized, "NGX library not initialized!");
		ZE_ASSERT(featureHandle && ngxParams, "Empty feature handles!");

		[[maybe_unused]] NVSDK_NGX_Result res;
		if constexpr (IS_DX12)
			res = NVSDK_NGX_D3D12_ReleaseFeature(featureHandle);
		else
			res = NVSDK_NGX_D3D11_ReleaseFeature(featureHandle);
		ZE_ASSERT(res == NVSDK_NGX_Result_Success, "Error releasing feature handle!");
		featureHandle = nullptr;

		if constexpr (IS_DX12)
			res = NVSDK_NGX_D3D12_DestroyParameters(ngxParams);
		else
			res = NVSDK_NGX_D3D11_DestroyParameters(ngxParams);
		ZE_ASSERT(res == NVSDK_NGX_Result_Success, "Error releasing feature parameters!");
		ngxParams = nullptr;
	}

	template<bool IS_DX12>
	bool NgxInterface<IS_DX12>::Init(void* d3dDevice, bool ignoreInternalMsg) noexcept
	{
		ZE_ASSERT(!initialized, "NGX library already initialized!");

		NVSDK_NGX_Result res = Initialize(d3dDevice);
		if (NVSDK_NGX_FAILED(res))
		{
			ZE_FAIL("Failed to initialize NGX library!");
			return false;
		}
		ignoreInternalLogs = ignoreInternalMsg;
		initialized = true;
		return true;
	}

	template<bool IS_DX12>
	void NgxInterface<IS_DX12>::Free(void* d3dDevice) noexcept
	{
		ZE_ASSERT(initialized, "NGX library already freed!");

		Shutdown(d3dDevice);
		initialized = false;
	}
#pragma endregion
}