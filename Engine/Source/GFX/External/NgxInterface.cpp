#if _ZE_NGX_ENABLED
#include "GFX/External/NgxInterface.h"
#include "GFX/CommandList.h"

namespace ZE::GFX::External
{
	constexpr const char* NgxInterface::GetFeatureSupportResult(NVSDK_NGX_Feature_Support_Result res) noexcept
	{
#define GET_SUPPORT_STRING(feature) case NVSDK_NGX_FeatureSupportResult_##feature: return #feature
		switch (res)
		{
			GET_SUPPORT_STRING(Supported);
			GET_SUPPORT_STRING(CheckNotPresent);
			GET_SUPPORT_STRING(DriverVersionUnsupported);
			GET_SUPPORT_STRING(AdapterUnsupported);
			GET_SUPPORT_STRING(OSVersionBelowMinimumSupported);
			GET_SUPPORT_STRING(NotImplemented);
		default: return "UNKNOWN";
		}
#undef GET_SUPPORT_STRING
	}

	constexpr const char* NgxInterface::GetFeatureString(NVSDK_NGX_Feature feature, FeatureString stringType) noexcept
	{
#define GET_STRING(featureName, scaleFactor) \
	case NVSDK_NGX_Feature_##featureName: \
	{ \
		switch (stringType) \
		{ \
		case FeatureString::Name: return #featureName; \
		case FeatureString::ScaleFactor: return scaleFactor; \
		case FeatureString::Available: return NVSDK_NGX_Parameter_##featureName##_Available; \
		case FeatureString::DriverUpdate: return NVSDK_NGX_Parameter_##featureName##_NeedsUpdatedDriver; \
		case FeatureString::DriverMinVersionMajor: return NVSDK_NGX_Parameter_##featureName##_MinDriverVersionMajor; \
		case FeatureString::DriverMinVersionMinor: return NVSDK_NGX_Parameter_##featureName##_MinDriverVersionMinor; \
		case FeatureString::InitResult: return NVSDK_NGX_Parameter_##featureName##_FeatureInitResult; \
		default: return ""; \
		} \
		break; \
	}
		switch (feature)
		{
			GET_STRING(SuperSampling, NVSDK_NGX_Parameter_SuperSampling_ScaleFactor)
				GET_STRING(InPainting, "")
				GET_STRING(ImageSuperResolution, "") // TODO: Add several scale factors
				GET_STRING(SlowMotion, "")
				GET_STRING(VideoSuperResolution, "")
				GET_STRING(ImageSignalProcessing, NVSDK_NGX_Parameter_ImageSignalProcessing_ScaleFactor)
				GET_STRING(DeepResolve, "")
		case NVSDK_NGX_Feature_FrameGeneration:
			{
				switch (stringType)
				{
				case FeatureString::Name: return "FrameGeneration";
				case FeatureString::DriverUpdate: return NVSDK_NGX_Parameter_FrameInterpolation_NeedsUpdatedDriver;
				case FeatureString::DriverMinVersionMajor: return NVSDK_NGX_Parameter_FrameInterpolation_MinDriverVersionMajor;
				case FeatureString::DriverMinVersionMinor: return NVSDK_NGX_Parameter_FrameInterpolation_FeatureInitResult;
				case FeatureString::ScaleFactor:
				case FeatureString::Available:
				case FeatureString::InitResult:
					ZE_FAIL("Feature string not available!");
					[[fallthrough]];
				default: return "";
				}
				break;
			}
		case NVSDK_NGX_Feature_DeepDVC:
		{
			switch (stringType)
			{
			case FeatureString::Name: return "DeepDVC";
			case FeatureString::ScaleFactor:
			case FeatureString::Available:
			case FeatureString::DriverUpdate:
			case FeatureString::DriverMinVersionMajor:
			case FeatureString::DriverMinVersionMinor:
			case FeatureString::InitResult:
				ZE_FAIL("Feature string not available!");
				[[fallthrough]];
			default: return "";
			}
			break;
		}
		case NVSDK_NGX_Feature_RayReconstruction:
		{
			switch (stringType)
			{
			case FeatureString::Name: return "RayReconstruction";
			case FeatureString::ScaleFactor:
			case FeatureString::Available:
			case FeatureString::DriverUpdate:
			case FeatureString::DriverMinVersionMajor:
			case FeatureString::DriverMinVersionMinor:
			case FeatureString::InitResult:
				ZE_FAIL("Feature string not available!");
				[[fallthrough]];
			default: return "";
			}
			break;
		}
		default:
			ZE_ENUM_UNHANDLED();
		case NVSDK_NGX_Feature_Reserved0:
		case NVSDK_NGX_Feature_Reserved1:
		case NVSDK_NGX_Feature_Reserved2:
		case NVSDK_NGX_Feature_Reserved3:
		case NVSDK_NGX_Feature_Reserved14:
		case NVSDK_NGX_Feature_Reserved15:
		case NVSDK_NGX_Feature_Reserved16:
		case NVSDK_NGX_Feature_Count:
		case NVSDK_NGX_Feature_Reserved_SDK:
		case NVSDK_NGX_Feature_Reserved_Core:
		case NVSDK_NGX_Feature_Reserved_Unknown:
			return "";
		}
#undef GET_STRING
	}

	void NVSDK_CONV NgxInterface::MessageHandler(const char* message, NVSDK_NGX_Logging_Level loggingLevel, NVSDK_NGX_Feature sourceComponent) noexcept
	{
		if (loggingLevel == NVSDK_NGX_LOGGING_LEVEL_OFF || (ignoreInternalLogs && (sourceComponent == NVSDK_NGX_Feature_Reserved_SDK || sourceComponent == NVSDK_NGX_Feature_Reserved_Core)))
			return;
		std::string feature = "";
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
		case NVSDK_NGX_Feature_RayReconstruction:
			feature = "[Ray Reconstruction] ";
			break;
		case NVSDK_NGX_Feature_Reserved14:
			feature = "[Reserved14] ";
			break;
		case NVSDK_NGX_Feature_Reserved15:
			feature = "[Reserved15] ";
			break;
		case NVSDK_NGX_Feature_Reserved16:
			feature = "[Reserved16] ";
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

	NVSDK_NGX_FeatureCommonInfo NgxInterface::GetCommonInfo() noexcept
	{
		NVSDK_NGX_FeatureCommonInfo commonInfo = {};
		commonInfo.PathListInfo.Path = nullptr;
		commonInfo.PathListInfo.Length = 0;
		commonInfo.InternalData = nullptr;
		commonInfo.LoggingInfo.LoggingCallback = MessageHandler;
		commonInfo.LoggingInfo.MinimumLoggingLevel = _ZE_MODE_PROFILE || _ZE_MODE_RELEASE ? NVSDK_NGX_LOGGING_LEVEL_OFF : NVSDK_NGX_LOGGING_LEVEL_ON;
		commonInfo.LoggingInfo.DisableOtherLoggingSinks = true;
		return commonInfo;
	}

	void NgxInterface::FreeScratchBuffer(NVSDK_NGX_Parameter* param) noexcept
	{
		auto it = scratchBuffersCache.find(param);
		if (it != scratchBuffersCache.end())
			scratchBuffersCache.erase(it);
	}

	NgxInterface::~NgxInterface()
	{
		ZE_ASSERT(scratchBuffersCache.size() == 0, "Not all NGX features were freed!");
		if (ngxCaps)
		{
			Status res = {};
			ZE_RHI_BACKEND_CALL_RET_VAR(res, DestroyParameter, ngxCaps);
			if (res)
			{
				ZE_CODE_ERROR(res, "Failed to free NGX capabilities!");
			}
		}
	}

	Expected<NgxInterface> NgxInterface::Create(Device& dev, bool ignoreInternalMsg) noexcept
	{
		NgxInterface ngx;
		auto createProxy = [&]() -> Expected<NgxInterface>
			{
				ZE_RHI_BACKEND_CREATE(External::NgxInterface, dev, GetCommonInfo());
			};
		auto expectedNgx = createProxy();
		if (!expectedNgx)
		{
			ZE_CODE_ERROR(expectedNgx.error(), "Failed to initialize NGX!");
			return std::unexpected(expectedNgx.error());
		}
		else
			ngx = std::move(*expectedNgx);

		Status code = ngx.GetCapabilities(ngx.ngxCaps);
		if (!code && ngx.ngxCaps)
		{
			NVSDK_NGX_Result res = ngx.ngxCaps->Get(NVSDK_NGX_Parameter_DLSSOptimalSettingsCallback, reinterpret_cast<void**>(&ngx.optimalSettingsCallback));
			if (NVSDK_NGX_SUCCEED(res) && ngx.optimalSettingsCallback)
			{
				ngx.ngxCaps->Set(NVSDK_NGX_Parameter_RTXValue, false); // Some older DLSS dlls still expect this value to be set
				ngx.ignoreInternalLogs = ignoreInternalMsg;
				return ngx;
			}
			else
			{
				code = ZE_NGX_ERROR(res);
				ZE_CODE_ERROR(code, "Cannot access DLSS callback for optimal settings!");
			}
		}
		else
		{
			ZE_CODE_ERROR(code, "Error getting NGX capabilities!");
		}
		return std::unexpected(code);
	}	

	Status NgxInterface::AllocateParameter(NVSDK_NGX_Parameter*& param) const noexcept
	{
		ZE_ASSERT(IsInitialized(), "NGX library not yet initialized!");

		Status res = {};
		ZE_RHI_BACKEND_CALL_RET_VAR(res, AllocateParameter, param);
		ZE_CODE_RET_FAILED(res);
		return {};
	}

	Status NgxInterface::CreateFeature(Device& dev, NVSDK_NGX_Feature type, NVSDK_NGX_Parameter* initParam, NVSDK_NGX_Handle*& feature) noexcept
	{
		ZE_ASSERT(IsInitialized(), "NGX library not yet initialized!");

		// In case of recreation of the feature with same parameter
		FreeScratchBuffer(initParam);

		U64 scratchBufferSize = 0;
		Status code = {};
		ZE_RHI_BACKEND_CALL_RET_VAR(code, GetScratchBufferSize, type, ngxCaps, scratchBufferSize);
		if (code)
		{
			ZE_CODE_ERROR(code, "Error getting NGX feature scratch buffer size!");
			return code;
		}

		initParam->Set(NVSDK_NGX_Parameter_RTXValue, false); // Some older DLSS dlls still expect this value to be set
		initParam->Set(NVSDK_NGX_Parameter_Scratch_SizeInBytes, scratchBufferSize);
		if (scratchBufferSize)
		{
			ZE_ASSERT(scratchBuffersCache.find(initParam) == scratchBuffersCache.end(), "Scratch buffer for this parameter already exists!");

			auto scratchBuffer = std::make_unique<U8[]>(scratchBufferSize);
			initParam->Set(NVSDK_NGX_Parameter_Scratch, scratchBuffer.get());
			scratchBuffersCache.emplace(initParam, std::move(scratchBuffer));
		}

		auto expCL = CommandList::Create(dev, QueueType::Compute);
		if (!expCL)
		{
			ZE_CODE_ERROR(expCL.error(), "Failed to create command list for NGX feature initialization!");
			return expCL.error();
		}

		CommandList cl = std::move(*expCL);
		code = cl.Open(dev);
		if (code)
		{
			ZE_CODE_ERROR(code, "Failed to open command list for NGX feature initialization!");
			return code;
		}

		ZE_RHI_BACKEND_CALL_RET_VAR(code, CreateFeature, dev, cl, type, initParam, feature);
		if (code)
		{
			ZE_CODE_ERROR(code, "Error creating NGX feature!");
			return code;
		}

		code = cl.Close(dev);
		if (code)
		{
			ZE_CODE_ERROR(code, "Failed to close command list after NGX feature initialization!");
			return code;
		}

		dev.ExecuteCompute(cl);
		auto expFence = dev.SetComputeFence();
		if (!expFence)
		{
			ZE_CODE_WARNING(expFence.error(), "Failed to set compute fence for NGX feature initialization, race condition may occur!");
		}
		else
		{
			code = dev.WaitCompute(*expFence);
			if (code)
			{
				ZE_CODE_WARNING(expFence.error(), "Failed to flush compute queue for NGX feature initialization, race condition may occur!");
			}
		}
		return {};
	}

	Status NgxInterface::RunFeature(Device& dev, CommandList& cl, const NVSDK_NGX_Handle* feature, const NVSDK_NGX_Parameter* param) const noexcept
	{
		ZE_ASSERT(IsInitialized(), "NGX library not yet initialized!");

		ZE_RHI_BACKEND_CALL_RET(EvaluateFeature, dev, cl, feature, param);
	}

	void NgxInterface::FreeParameter(NVSDK_NGX_Parameter* param) noexcept
	{
		ZE_ASSERT(IsInitialized(), "NGX library not yet initialized!");
		ZE_ASSERT(param, "Empty NGX parameter!");

		// In case param was used to create a feature
		FreeScratchBuffer(param);

		Status code = {};
		ZE_RHI_BACKEND_CALL_RET_VAR(code, DestroyParameter, param);
		if (code)
		{
			ZE_CODE_ERROR(code, "Failed to destroy NGX param!");
		}
	}

	void NgxInterface::FreeFeature(NVSDK_NGX_Handle* feature) const noexcept
	{
		ZE_ASSERT(IsInitialized(), "NGX library not yet initialized!");
		ZE_ASSERT(feature, "Empty NGX feature!");

		Status code = {};
		ZE_RHI_BACKEND_CALL_RET_VAR(code, ReleaseFeature, feature);
		if (code)
		{
			ZE_CODE_ERROR(code, "Failed to free NGX feature!");
		}
	}

	bool NgxInterface::IsFeatureAvailable(Device& dev, NVSDK_NGX_Feature feature) const noexcept
	{
		ZE_ASSERT(IsInitialized(), "NGX library not yet initialized!");

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
		Status code = {};
		ZE_RHI_BACKEND_CALL_RET_VAR(code, GetFeatureRequirements, dev, info, supported);

		if (code)
		{
			ZE_CODE_ERROR(code, "[" + std::string(GetFeatureString(feature, FeatureString::Name)) + "] Cannot feature requirements!");
		}
		else
		{
			if (supported.FeatureSupported == NVSDK_NGX_FeatureSupportResult_Supported)
			{
				S32 needsUpdatedDriver = 0;
				NVSDK_NGX_Result res = ngxCaps->Get(GetFeatureString(feature, FeatureString::DriverUpdate), &needsUpdatedDriver);
				if (NVSDK_NGX_SUCCEED(res))
				{
					if (needsUpdatedDriver)
					{
						U32 minDriverVersionMajor = 0;
						U32 minDriverVersionMinor = 0;
						res = ngxCaps->Get(GetFeatureString(feature, FeatureString::DriverMinVersionMajor), &minDriverVersionMajor);
						ngxCaps->Get(GetFeatureString(feature, FeatureString::DriverMinVersionMinor), &minDriverVersionMinor);
						if (NVSDK_NGX_SUCCEED(res))
							Logger::Warning("[NGX] [" + std::string(GetFeatureString(feature, FeatureString::Name)) + "] Feature not available due to outdated driver! Minimum supported driver: " +
								std::to_string(minDriverVersionMajor) + "." + std::to_string(minDriverVersionMinor));
						else
							Logger::Warning("[NGX] [" + std::string(GetFeatureString(feature, FeatureString::Name)) + "] Feature not available due to outdated driver!");
						return false;
					}
				}
				else
				{
					ZE_CODE_WARNING(ZE_NGX_ERROR(res), "[" + std::string(GetFeatureString(feature, FeatureString::Name)) + "] Cannot check if driver need updating!");
				}
				S32 featureAvailable = 0;
				if (NVSDK_NGX_SUCCEED(ngxCaps->Get(GetFeatureString(feature, FeatureString::Available), &featureAvailable)))
					return featureAvailable != 0;
			}
			else
			{
				switch (supported.FeatureSupported)
				{
				case NVSDK_NGX_FeatureSupportResult_AdapterUnsupported:
				{
					Logger::Warning("[NGX] [" + std::string(GetFeatureString(feature, FeatureString::Name)) +
						"] Feature not supported due to oudated GPU, minimum supported architecture (according to NV_GPU_ARCHITECTURE_ID in NvAPI): " +
						std::to_string(supported.MinHWArchitecture));
					break;
				}
				case NVSDK_NGX_FeatureSupportResult_OSVersionBelowMinimumSupported:
				{
					Logger::Warning("[NGX] [" + std::string(GetFeatureString(feature, FeatureString::Name)) + "] Feature not supported due to outdated OS, minimum supported version: " + supported.MinOSVersion);
					break;
				}
				case NVSDK_NGX_FeatureSupportResult_CheckNotPresent:
				case NVSDK_NGX_FeatureSupportResult_DriverVersionUnsupported:
				case NVSDK_NGX_FeatureSupportResult_NotImplemented:
				{
					Logger::Warning("[NGX] [" + std::string(GetFeatureString(feature, FeatureString::Name)) + "] Reason for not supported feature: " + GetFeatureSupportResult(supported.FeatureSupported));
					break;
				}
				default:
					ZE_ENUM_UNHANDLED();
				case NVSDK_NGX_FeatureSupportResult_Supported:
				{
					Logger::Warning("[NGX] [" + std::string(GetFeatureString(feature, FeatureString::Name)) + "] Unknown reason for not supported feature!");
					break;
				}
				}
			}
		}
		return false;
	}

	UInt2 NgxInterface::GetRenderSize(UInt2 targetSize, NVSDK_NGX_PerfQuality_Value quality) noexcept
	{
		ZE_ASSERT(IsInitialized(), "NGX library not yet initialized!");

		// Query for optimal sizes
		ngxCaps->Set(NVSDK_NGX_Parameter_Width, targetSize.X);
		ngxCaps->Set(NVSDK_NGX_Parameter_Height, targetSize.Y);
		ngxCaps->Set(NVSDK_NGX_Parameter_PerfQualityValue, quality);

		NVSDK_NGX_Result res = optimalSettingsCallback(ngxCaps);
		UInt2 renderSize = targetSize;
		if (NVSDK_NGX_SUCCEED(res))
		{
			if (NVSDK_NGX_FAILED(ngxCaps->Get(NVSDK_NGX_Parameter_OutWidth, &renderSize.X))
				|| NVSDK_NGX_FAILED(ngxCaps->Get(NVSDK_NGX_Parameter_OutHeight, &renderSize.Y)))
			{
				ZE_FAIL("Failed to get optimal render sizes for DLSS!");
				renderSize = targetSize;
			}
		}
		else
		{
			ZE_CODE_ERROR(ZE_NGX_ERROR(res), "Error retrieving optimal render sizes for DLSS!");
		}
		return renderSize;
	}
}
#endif

// What should be investigated for extended support
#if 0

#define NVSDK_NGX_Parameter_OptLevel "Snippet.OptLevel"
#define NVSDK_NGX_Parameter_IsDevSnippetBranch "Snippet.IsDevBranch"
#define NVSDK_NGX_Parameter_ImageSuperResolution_ScaleFactor_2_1 "ImageSuperResolution.ScaleFactor.2.1"
#define NVSDK_NGX_Parameter_ImageSuperResolution_ScaleFactor_3_1 "ImageSuperResolution.ScaleFactor.3.1"
#define NVSDK_NGX_Parameter_ImageSuperResolution_ScaleFactor_3_2 "ImageSuperResolution.ScaleFactor.3.2"
#define NVSDK_NGX_Parameter_ImageSuperResolution_ScaleFactor_4_3 "ImageSuperResolution.ScaleFactor.4.3"
#define NVSDK_NGX_Parameter_NumFrames "NumFrames"
#define NVSDK_NGX_Parameter_Scale "Scale"
#define NVSDK_NGX_Parameter_Scratch "Scratch"
#define NVSDK_NGX_Parameter_Scratch_SizeInBytes "Scratch.SizeInBytes"
#define NVSDK_NGX_Parameter_Input1 "Input1"
#define NVSDK_NGX_Parameter_Input1_Format "Input1.Format"
#define NVSDK_NGX_Parameter_Input1_SizeInBytes "Input1.SizeInBytes"
#define NVSDK_NGX_Parameter_Input2 "Input2"
#define NVSDK_NGX_Parameter_Input2_Format "Input2.Format"
#define NVSDK_NGX_Parameter_Input2_SizeInBytes "Input2.SizeInBytes"
#define NVSDK_NGX_Parameter_Color "Color"
#define NVSDK_NGX_Parameter_Color_Format "Color.Format"
#define NVSDK_NGX_Parameter_Color_SizeInBytes "Color.SizeInBytes"
#define NVSDK_NGX_Parameter_FI_Color1 "Color1"
#define NVSDK_NGX_Parameter_FI_Color2 "Color2"
#define NVSDK_NGX_Parameter_Albedo "Albedo"
#define NVSDK_NGX_Parameter_Output_Format "Output.Format"
#define NVSDK_NGX_Parameter_Output_SizeInBytes "Output.SizeInBytes"
#define NVSDK_NGX_Parameter_FI_Output1 "Output1"
#define NVSDK_NGX_Parameter_FI_Output2 "Output2"
#define NVSDK_NGX_Parameter_FI_Output3 "Output3"
#define NVSDK_NGX_Parameter_BlendFactor "BlendFactor"
#define NVSDK_NGX_Parameter_FI_MotionVectors1 "MotionVectors1"
#define NVSDK_NGX_Parameter_FI_MotionVectors2 "MotionVectors2"
#define NVSDK_NGX_Parameter_Rect_X "Rect.X"
#define NVSDK_NGX_Parameter_Rect_Y "Rect.Y"
#define NVSDK_NGX_Parameter_Rect_W "Rect.W"
#define NVSDK_NGX_Parameter_Rect_H "Rect.H"
#define NVSDK_NGX_Parameter_OutRect_X "OutRect.X"
#define NVSDK_NGX_Parameter_OutRect_Y "OutRect.Y"
#define NVSDK_NGX_Parameter_OutRect_W "OutRect.W"
#define NVSDK_NGX_Parameter_OutRect_H "OutRect.H"
#define NVSDK_NGX_Parameter_Model "Model"
#define NVSDK_NGX_Parameter_Format "Format"
#define NVSDK_NGX_Parameter_SizeInBytes "SizeInBytes"
#define NVSDK_NGX_Parameter_ResourceAllocCallback      "ResourceAllocCallback"
#define NVSDK_NGX_Parameter_BufferAllocCallback        "BufferAllocCallback"
#define NVSDK_NGX_Parameter_Tex2DAllocCallback         "Tex2DAllocCallback"
#define NVSDK_NGX_Parameter_ResourceReleaseCallback    "ResourceReleaseCallback"
#define NVSDK_NGX_Parameter_Hint_UseFireflySwatter "Hint.UseFireflySwatter"
#define NVSDK_NGX_Parameter_Resource_Width "ResourceWidth"
#define NVSDK_NGX_Parameter_Resource_Height "ResourceHeight"
#define NVSDK_NGX_Parameter_Resource_OutWidth "ResourceOutWidth"
#define NVSDK_NGX_Parameter_Resource_OutHeight "ResourceOutHeight"
#define NVSDK_NGX_Parameter_FI_Depth1 "Depth1"
#define NVSDK_NGX_Parameter_FI_Depth2 "Depth2"
#define NVSDK_NGX_Parameter_DLSSGetStatsCallback    "DLSSGetStatsCallback"
#define NVSDK_NGX_Parameter_FI_Mode     "FIMode"
#define NVSDK_NGX_Parameter_FI_OF_Preset    "FIOFPreset"
#define NVSDK_NGX_Parameter_FI_OF_GridSize  "FIOFGridSize"
#define NVSDK_NGX_Parameter_Denoise "Denoise"
#define NVSDK_NGX_Parameter_DLSS_Checkerboard_Jitter_Hack "DLSS.Checkerboard.Jitter.Hack"
#define NVSDK_NGX_Parameter_FreeMemOnReleaseFeature "FreeMemOnReleaseFeature"
#define NVSDK_NGX_Parameter_AnimatedTextureMask "AnimatedTextureMask"
#define NVSDK_NGX_Parameter_DepthHighRes "DepthHighRes"
#define NVSDK_NGX_Parameter_Position_ViewSpace "Position.ViewSpace"
#define NVSDK_NGX_Parameter_DLSS_INV_VIEW_PROJECTION_MATRIX "InvViewProjectionMatrix"
#define NVSDK_NGX_Parameter_DLSS_CLIP_TO_PREV_CLIP_MATRIX   "ClipToPrevClipMatrix"

#define NVSDK_NGX_Parameter_DLSS_Get_Dynamic_Max_Render_Width     "DLSS.Get.Dynamic.Max.Render.Width"
#define NVSDK_NGX_Parameter_DLSS_Get_Dynamic_Max_Render_Height    "DLSS.Get.Dynamic.Max.Render.Height"
#define NVSDK_NGX_Parameter_DLSS_Get_Dynamic_Min_Render_Width     "DLSS.Get.Dynamic.Min.Render.Width"
#define NVSDK_NGX_Parameter_DLSS_Get_Dynamic_Min_Render_Height    "DLSS.Get.Dynamic.Min.Render.Height"

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_UpdateFeature(const NVSDK_NGX_Application_Identifier* ApplicationId, const NVSDK_NGX_Feature FeatureID);

#pragma region nvsdk_ngx_vk

NVSDK_NGX_API NVSDK_NGX_Result  NVSDK_CONV NVSDK_NGX_VULKAN_RequiredExtensions(unsigned int* OutInstanceExtCount, const char*** OutInstanceExts, unsigned int* OutDeviceExtCount, const char*** OutDeviceExts);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_GetFeatureInstanceExtensionRequirements(const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo,
	uint32_t* OutExtensionCount,
	VkExtensionProperties** OutExtensionProperties);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_GetFeatureDeviceExtensionRequirements(VkInstance Instance,
	VkPhysicalDevice PhysicalDevice,
	const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo,
	uint32_t* OutExtensionCount,
	VkExtensionProperties** OutExtensionProperties);

#pragma endregion
#endif