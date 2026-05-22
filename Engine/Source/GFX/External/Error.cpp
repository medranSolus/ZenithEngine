#include "GFX/External/Error.h"

namespace ZE::GFX::External::Error
{
	std::string Ffx::message(int condition) const
	{
		switch (static_cast<FfxErrorCode>(condition))
		{
		default:
			ZE_FAIL("Unknown FFX error code!");
			return "Unknown FFX error code!";
		case FFX_OK:
			return "Success";
		case FFX_ERROR_INVALID_POINTER:
			return "Invalid pointer";
		case FFX_ERROR_INVALID_ALIGNMENT:
			return "Invalid alignment";
		case FFX_ERROR_INVALID_SIZE:
			return "Invalid size";
		case FFX_EOF:
			return "The end of the file was encountered";
		case FFX_ERROR_INVALID_PATH:
			return "Specified path was invalid";
		case FFX_ERROR_EOF:
			return "The operation failed because end of file was reached";
		case FFX_ERROR_MALFORMED_DATA:
			return "Malformed data";
		case FFX_ERROR_OUT_OF_MEMORY:
			return "Out of memory";
		case FFX_ERROR_INCOMPLETE_INTERFACE:
			return "Interface was not fully configured";
		case FFX_ERROR_INVALID_ENUM:
			return "Invalid enumeration value";
		case FFX_ERROR_INVALID_ARGUMENT:
			return "Invalid argument";
		case FFX_ERROR_OUT_OF_RANGE:
			return "Value out of range";
		case FFX_ERROR_NULL_DEVICE:
			return "Null device";
		case FFX_ERROR_BACKEND_API_ERROR:
			return "Backend API returned error";
		case FFX_ERROR_INSUFFICIENT_MEMORY:
			return "Not enough memory";
		case FFX_ERROR_INVALID_VERSION:
			return "Wrong backend was linked";
		case FFX_ERROR_ACCESS_DENIED:
			return "Access to the resource was denied";
		}
	}

	std::string HBAO::message(int condition) const
	{
		switch (static_cast<GFSDK_SSAO_Status>(condition))
		{
		default:
			ZE_FAIL("Unknown HBAO+ error code!");
			return "Unknown HBAO+ error code!";
		case GFSDK_SSAO_OK:
			return "Success";
		case GFSDK_SSAO_VERSION_MISMATCH:
			return "The header version number does not match the DLL version number";
		case GFSDK_SSAO_NULL_ARGUMENT:
			return "One of the required argument pointers is NULL";
		case GFSDK_SSAO_INVALID_PROJECTION_MATRIX:
			return "The projection matrix is not valid";
		case GFSDK_SSAO_INVALID_WORLD_TO_VIEW_MATRIX:
			return "The world-to-view matrix is not valid (transposing it may help)";
		case GFSDK_SSAO_INVALID_NORMAL_TEXTURE_RESOLUTION:
			return "The normal-texture resolution does not match the depth-texture resolution";
		case GFSDK_SSAO_INVALID_NORMAL_TEXTURE_SAMPLE_COUNT:
			return "The normal-texture sample count does not match the depth-texture sample count";
		case GFSDK_SSAO_INVALID_VIEWPORT_DIMENSIONS:
			return "One of the viewport dimensions (width or height) is 0";
		case GFSDK_SSAO_INVALID_VIEWPORT_DEPTH_RANGE:
			return "The viewport depth range is not a sub-range of [0.f,1.f]";
		case GFSDK_SSAO_INVALID_SECOND_DEPTH_TEXTURE_RESOLUTION:
			return "The resolution of the second depth texture does not match the one of the first depth texture";
		case GFSDK_SSAO_INVALID_SECOND_DEPTH_TEXTURE_SAMPLE_COUNT:
			return "The sample count of the second depth texture does not match the one of the first depth texture";
		case GFSDK_SSAO_MEMORY_ALLOCATION_FAILED:
			return "Failed to allocate memory on the heap";
		case GFSDK_SSAO_INVALID_DEPTH_STENCIL_RESOLUTION:
			return "The depth-stencil resolution does not match the output render-target resolution";
		case GFSDK_SSAO_INVALID_DEPTH_STENCIL_SAMPLE_COUNT:
			return "The depth-stencil sample count does not match the output render-target sample count";
		case GFSDK_SSAO_D3D_FEATURE_LEVEL_NOT_SUPPORTED:
			return "The current D3D11 feature level is lower than 11_0";
		case GFSDK_SSAO_D3D_RESOURCE_CREATION_FAILED:
			return "A resource-creation call has failed (running out of memory?)";
		case GFSDK_SSAO_D3D12_UNSUPPORTED_DEPTH_CLAMP_MODE:
			return "CLAMP_TO_BORDER is used (implemented on D3D11 & GL, but not on D3D12)";
		case GFSDK_SSAO_D3D12_INVALID_HEAP_TYPE:
			return "One of the heaps provided to GFSDK_SSAO_CreateContext_D3D12 has an unexpected type";
		case GFSDK_SSAO_D3D12_INSUFFICIENT_DESCRIPTORS:
			return "One of the heaps provided to GFSDK_SSAO_CreateContext_D3D12 has an insufficient number of descriptors";
		case GFSDK_SSAO_D3D12_INVALID_NODE_MASK:
			return "NodeMask has more than one bit set. HBAO+ only supports operation on one D3D12 device node";
		case GFSDK_SSAO_NO_SECOND_LAYER_PROVIDED:
			return "FullResDepthTexture2ndLayerSRV is not set, but DualLayerAO is enabled";
		}
	}

#if _ZE_FFX_API_ENABLED
	std::string FfxApi::message(int condition) const
	{
		switch (static_cast<ffxReturnCode_t>(condition))
		{
		default:
			ZE_FAIL("Unknown FFX API error code!");
			return "Unknown FFX API error code!";
		case FFX_API_RETURN_OK:
			return "Success";
		case FFX_API_RETURN_ERROR:
			return "Generic error";
		case FFX_API_RETURN_ERROR_UNKNOWN_DESCTYPE:
			return "Unknown structure type used";
		case FFX_API_RETURN_ERROR_RUNTIME_ERROR:
			return "Runtime backend error";
		case FFX_API_RETURN_NO_PROVIDER:
			return "No provider for given structure type";
		case FFX_API_RETURN_ERROR_MEMORY:
			return "Memory allocation failed";
		case FFX_API_RETURN_ERROR_PARAMETER:
			return "Invalid parameter";
		case FFX_API_RETURN_PROVIDER_NO_SUPPORT_NEW_DESCTYPE:
			return "New structure not supported in old DLL";
		}
	}
#endif
#if _ZE_NGX_ENABLED
	std::string NGX::message(int condition) const
	{
		switch (static_cast<NVSDK_NGX_Result>(condition == 0 ? NVSDK_NGX_Result_Success : condition))
		{
		default:
			ZE_FAIL("Unknown NGX error code!");
			return "Unknown NGX error code!";
		case NVSDK_NGX_Result_Success:
			return "Success";
		case NVSDK_NGX_Result_Fail:
			return "Fail";
		case NVSDK_NGX_Result_FAIL_FeatureNotSupported:
			return "Feature not supported on current hardware";
		case NVSDK_NGX_Result_FAIL_PlatformError:
			return "Platform error, check logs for more information";
		case NVSDK_NGX_Result_FAIL_FeatureAlreadyExists:
			return "Feature with given parameters already exists";
		case NVSDK_NGX_Result_FAIL_FeatureNotFound:
			return "Feature handle does not exist";
		case NVSDK_NGX_Result_FAIL_InvalidParameter:
			return "Invalid parameter";
		case NVSDK_NGX_Result_FAIL_ScratchBufferTooSmall:
			return "Provided buffer too small (see NVSDK_NGX_GetScratchBufferSize)";
		case NVSDK_NGX_Result_FAIL_NotInitialized:
			return "SDK was not initialized properly";
		case NVSDK_NGX_Result_FAIL_UnsupportedInputFormat:
			return "Unsupported format used for input buffers";
		case NVSDK_NGX_Result_FAIL_RWFlagMissing:
			return "Missing RW (UAV) access input/output feature buffer (DX11/DX12 specific)";
		case NVSDK_NGX_Result_FAIL_MissingInput:
			return "Missing required feature inputs";
		case NVSDK_NGX_Result_FAIL_UnableToInitializeFeature:
			return "Feature is unavailable";
		case NVSDK_NGX_Result_FAIL_OutOfDate:
			return "NGX system libraries are old and need an update";
		case NVSDK_NGX_Result_FAIL_OutOfGPUMemory:
			return "Not enough GPU memory for feature";
		case NVSDK_NGX_Result_FAIL_UnsupportedFormat:
			return "Input format not supported by feature";
		case NVSDK_NGX_Result_FAIL_UnableToWriteToAppDataPath:
			return "Path provided in InApplicationDataPath cannot be written to";
		case NVSDK_NGX_Result_FAIL_UnsupportedParameter:
			return "Unsupported parameter was provided";
		case NVSDK_NGX_Result_FAIL_Denied:
			return "Feature or application was denied (contact NVIDIA for further details)";
		case NVSDK_NGX_Result_FAIL_NotImplemented:
			return "Feature or functionality is not implemented";
		}
	}
#endif
#if _ZE_XESS_ENABLED
	std::string XeSS::message(int condition) const
	{
		switch (static_cast<xess_result_t>(condition))
		{
		default:
			ZE_FAIL("Unknown XeSS error code!");
			return "Unknown XeSS error code!";
		case XESS_RESULT_WARNING_NONEXISTING_FOLDER:
			return "Warning, dump folder not exists, skipping write";
		case XESS_RESULT_WARNING_OLD_DRIVER:
			return "Outdated driver";
		case XESS_RESULT_SUCCESS:
			return "Success";
		case XESS_RESULT_ERROR_UNSUPPORTED_DEVICE:
			return "XeSS not supported";
		case XESS_RESULT_ERROR_UNSUPPORTED_DRIVER:
			return "Unsupported driver";
		case XESS_RESULT_ERROR_UNINITIALIZED:
			return "Uninitialized";
		case XESS_RESULT_ERROR_INVALID_ARGUMENT:
			return "Invalid argument";
		case XESS_RESULT_ERROR_DEVICE_OUT_OF_MEMORY:
			return "Not enough available GPU memory";
		case XESS_RESULT_ERROR_DEVICE:
			return "Device function";
		case XESS_RESULT_ERROR_NOT_IMPLEMENTED:
			return "Function is not implemented";
		case XESS_RESULT_ERROR_INVALID_CONTEXT:
			return "Invalid context";
		case XESS_RESULT_ERROR_OPERATION_IN_PROGRESS:
			return "Operation not finished";
		case XESS_RESULT_ERROR_UNSUPPORTED:
			return "Operation not supported";
		case XESS_RESULT_ERROR_CANT_LOAD_LIBRARY:
			return "Cannot load library";
		case XESS_RESULT_ERROR_WRONG_CALL_ORDER:
			return "Invalid function order";
		case XESS_RESULT_ERROR_UNKNOWN:
			return "Unknown internal failure";
		}
	}
#endif
}