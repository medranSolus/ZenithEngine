#include "GFX/NgxInterface.h"
#include "GFX/CommandList.h"

namespace ZE::GFX
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

	constexpr const char* NgxInterface::GetResultString(NVSDK_NGX_Result res) noexcept
	{
#define GET_FAIL_STRING(feature, msg) case NVSDK_NGX_Result_FAIL_##feature: return #feature msg
		switch (res)
		{
		case NVSDK_NGX_Result_Success: return "Success";
		case NVSDK_NGX_Result_Fail: return "Fail";
			GET_FAIL_STRING(FeatureNotSupported, ", feature is not supported on current hardware");
			GET_FAIL_STRING(PlatformError, ", check API debug layer log for more information");
			GET_FAIL_STRING(FeatureAlreadyExists, ", feature with given parameters already exists");
			GET_FAIL_STRING(FeatureNotFound, ", feature with provided handle does not exist");
			GET_FAIL_STRING(InvalidParameter, ", invalid parameter was provided");
			GET_FAIL_STRING(ScratchBufferTooSmall, ", provided buffer is too small, please use size provided by NVSDK_NGX_GetScratchBufferSize");
			GET_FAIL_STRING(NotInitialized, ", SDK was not initialized properly");
			GET_FAIL_STRING(UnsupportedInputFormat, ", unsupported format used for input/output buffers");
			GET_FAIL_STRING(RWFlagMissing, ", feature input/output needs RW access (UAV) (D3D11/D3D12 specific)");
			GET_FAIL_STRING(MissingInput, ", feature was created with specific input but none is provided at evaluation");
			GET_FAIL_STRING(UnableToInitializeFeature, ", feature is not available on the system");
			GET_FAIL_STRING(OutOfDate, ", NGX system libraries are old and need an update");
			GET_FAIL_STRING(OutOfGPUMemory, ", feature requires more GPU memory than it is available on system");
			GET_FAIL_STRING(UnsupportedFormat, ", format used in input buffer(s) is not supported by feature");
			GET_FAIL_STRING(UnableToWriteToAppDataPath, ", path provided in InApplicationDataPath cannot be written to");
			GET_FAIL_STRING(UnsupportedParameter, ", unsupported parameter was provided (e.g. specific scaling factor is unsupported)");
			GET_FAIL_STRING(Denied, ", the feature or application was denied (contact NVIDIA for further details)");
			GET_FAIL_STRING(NotImplemented, ", the feature or functionality is not implemented");
		default: return "UNKNOWN";
		}
#undef GET_FAIL_STRING
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
		commonInfo.LoggingInfo.MinimumLoggingLevel = _ZE_MODE_PROFILE || _ZE_MODE_RELEASE ? NVSDK_NGX_LOGGING_LEVEL_OFF : (_ZE_MODE_DEV ? NVSDK_NGX_LOGGING_LEVEL_ON : NVSDK_NGX_LOGGING_LEVEL_VERBOSE);
		commonInfo.LoggingInfo.DisableOtherLoggingSinks = true;
		return commonInfo;
	}

	void NgxInterface::FreeScratchBuffer(NVSDK_NGX_Parameter* param) noexcept
	{
		U64 scratchBufferSize = 0;
		NVSDK_NGX_Result res = param->Get(NVSDK_NGX_Parameter_Scratch_SizeInBytes, &scratchBufferSize);
		if (NVSDK_NGX_SUCCEED(res) && scratchBufferSize)
		{
			U8* scratchBuffer = nullptr;
			res = param->Get(NVSDK_NGX_Parameter_Scratch, reinterpret_cast<void**>(&scratchBuffer));
			ZE_ASSERT(NVSDK_NGX_SUCCEED(res), "Failed to get scratch buffer pointer, possible memory leak! Error: " + std::string(GetResultString(res)));
			if (scratchBuffer)
				delete[] scratchBuffer;
		}
	}

	bool NgxInterface::Init(Device& dev, bool ignoreInternalMsg) noexcept
	{
		ZE_ASSERT(!IsInitialized(), "NGX library already initialized!");
		ZE_RHI_BACKEND_VAR.Init();

		NVSDK_NGX_Result res = NVSDK_NGX_Result_Success;
		ZE_RHI_BACKEND_CALL_RET(res, InitNGX, dev, GetCommonInfo());
		if (NVSDK_NGX_SUCCEED(res))
		{
			ZE_RHI_BACKEND_CALL_RET(res, GetCapabilities, ngxCaps);
			if (NVSDK_NGX_SUCCEED(res) && ngxCaps)
			{
				res = ngxCaps->Get(NVSDK_NGX_Parameter_DLSSOptimalSettingsCallback, reinterpret_cast<void**>(&optimalSettingsCallback));
				if (NVSDK_NGX_SUCCEED(res) && optimalSettingsCallback)
				{
					ngxCaps->Set(NVSDK_NGX_Parameter_RTXValue, false); // Some older DLSS dlls still expect this value to be set
					ignoreInternalLogs = ignoreInternalMsg;
					return true;
				}
				else
				{
					ZE_FAIL("Cannot access DLSS callback for optimal settings! Error: " + std::string(GetResultString(res)));
				}

				ZE_RHI_BACKEND_CALL_RET(res, DestroyParameter, ngxCaps);
				ZE_ASSERT(NVSDK_NGX_SUCCEED(res), "Failed to free NGX capabilities! Error: " + std::string(GetResultString(res)));
				ngxCaps = nullptr;
			}
			else
			{
				ZE_FAIL("Error getting NGX parameters! Error: " + std::string(GetResultString(res)));
			}
			ZE_RHI_BACKEND_CALL_RET(res, Shutdown, dev);
			ZE_ASSERT(NVSDK_NGX_SUCCEED(res), "Failed to free NGX library! Error: " + std::string(GetResultString(res)));
		}
		else
		{
			ZE_FAIL("Failed to initialize NGX! Error: " + std::string(GetResultString(res)));
		}
		return false;
	}

	void NgxInterface::Free(Device& dev) noexcept
	{
		ZE_ASSERT(IsInitialized(), "NGX library already freed!");

		[[maybe_unused]] NVSDK_NGX_Result res = NVSDK_NGX_Result_Success;
		ZE_RHI_BACKEND_CALL_RET(res, DestroyParameter, ngxCaps);
		ZE_ASSERT(NVSDK_NGX_SUCCEED(res), "Failed to free NGX capabilities! Error: " + std::string(GetResultString(res)));
		ngxCaps = nullptr;

		ZE_RHI_BACKEND_CALL_RET(res, Shutdown, dev);
		ZE_ASSERT(NVSDK_NGX_SUCCEED(res), "Failed to free NGX library! Error: " + std::string(GetResultString(res)));
	}

	NVSDK_NGX_Parameter* NgxInterface::AllocateParameter() const noexcept
	{
		ZE_ASSERT(IsInitialized(), "NGX library not yet initialized!");

		NVSDK_NGX_Parameter* param = nullptr;
		[[maybe_unused]] NVSDK_NGX_Result res = NVSDK_NGX_Result_Success;
		ZE_RHI_BACKEND_CALL_RET(res, AllocateParameter, param);
		ZE_ASSERT(NVSDK_NGX_SUCCEED(res), "Failed to allocate param! Error: " + std::string(GetResultString(res)));
		return param;
	}

	NVSDK_NGX_Handle* NgxInterface::CreateFeature(Device& dev, NVSDK_NGX_Feature feature, NVSDK_NGX_Parameter* initParam) const noexcept
	{
		ZE_ASSERT(IsInitialized(), "NGX library not yet initialized!");

		// In case of recreation of the feature with same parameter
		FreeScratchBuffer(initParam);

		U64 scratchBufferSize = 0;
		NVSDK_NGX_Result res = NVSDK_NGX_Result_Success;
		ZE_RHI_BACKEND_CALL_RET(res, GetScratchBufferSize, feature, ngxCaps, scratchBufferSize);
		if (NVSDK_NGX_FAILED(res))
		{
			ZE_FAIL("Error getting NGX feature scratch buffer size! Error: " + std::string(GetResultString(res)));
			return nullptr;
		}
		initParam->Set(NVSDK_NGX_Parameter_RTXValue, false); // Some older DLSS dlls still expect this value to be set
		initParam->Set(NVSDK_NGX_Parameter_Scratch_SizeInBytes, scratchBufferSize);
		if (scratchBufferSize)
			initParam->Set(NVSDK_NGX_Parameter_Scratch, new U8[scratchBufferSize]);

		NVSDK_NGX_Handle* featureHandle = nullptr;
		CommandList cl(dev, QueueType::Compute);
		cl.Open(dev);
		ZE_RHI_BACKEND_CALL_RET(res, CreateFeature, dev, cl, feature, initParam, featureHandle);
		cl.Close(dev);
		if (NVSDK_NGX_FAILED(res))
		{
			ZE_FAIL("Error getting initializing NGX feature! Error: " + std::string(GetResultString(res)));
			return nullptr;
		}
		dev.ExecuteCompute(cl);
		dev.WaitCompute(dev.SetComputeFence());
		cl.Free(dev);
		return featureHandle;
	}

	bool NgxInterface::RunFeature(Device& dev, CommandList& cl, const NVSDK_NGX_Handle* feature, const NVSDK_NGX_Parameter* param) const noexcept
	{
		ZE_ASSERT(IsInitialized(), "NGX library not yet initialized!");

		NVSDK_NGX_Result res = NVSDK_NGX_Result_Success;
		ZE_RHI_BACKEND_CALL_RET(res, EvaluateFeature, dev, cl, feature, param);
		return NVSDK_NGX_SUCCEED(res);
	}

	void NgxInterface::FreeParameter(NVSDK_NGX_Parameter* param) const noexcept
	{
		ZE_ASSERT(IsInitialized(), "NGX library not yet initialized!");
		ZE_ASSERT(param, "Empty NGX parameter!");

		// In case param was used to create a feature
		FreeScratchBuffer(param);

		[[maybe_unused]] NVSDK_NGX_Result res = NVSDK_NGX_Result_Success;
		ZE_RHI_BACKEND_CALL_RET(res, DestroyParameter, param);
		ZE_ASSERT(NVSDK_NGX_SUCCEED(res), "Failed to destroy NGX param! Error: " + std::string(GetResultString(res)));
	}

	void NgxInterface::FreeFeature(NVSDK_NGX_Handle* feature) const noexcept
	{
		ZE_ASSERT(IsInitialized(), "NGX library not yet initialized!");
		ZE_ASSERT(feature, "Empty NGX feature!");

		[[maybe_unused]] NVSDK_NGX_Result res = NVSDK_NGX_Result_Success;
		ZE_RHI_BACKEND_CALL_RET(res, ReleaseFeature, feature);
		ZE_ASSERT(NVSDK_NGX_SUCCEED(res), "Failed to destroy NGX feature! Error: " + std::string(GetResultString(res)));
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
		NVSDK_NGX_Result res = NVSDK_NGX_Result_Success;
		ZE_RHI_BACKEND_CALL_RET(res, GetFeatureRequirements, dev, info, supported);

		if (NVSDK_NGX_SUCCEED(res))
		{
			if (supported.FeatureSupported == NVSDK_NGX_FeatureSupportResult_Supported)
			{
				S32 needsUpdatedDriver = 0;
				res = ngxCaps->Get(GetFeatureString(feature, FeatureString::DriverUpdate), &needsUpdatedDriver);
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
					Logger::Warning("[NGX] [" + std::string(GetFeatureString(feature, FeatureString::Name)) + "] Cannot check if driver need updating!");
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
		else
			Logger::Warning("[NGX] [" + std::string(GetFeatureString(feature, FeatureString::Name)) + "] Cannot feature requirements!");

		return false;
	}

	UInt2 NgxInterface::GetRenderSize(UInt2 targetSize, NVSDK_NGX_PerfQuality_Value quality) const noexcept
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
			ZE_FAIL("Error retrieving optimal render sizes for DLSS! Error: " + std::string(GetResultString(res)));
		}
		return renderSize;
	}
}

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