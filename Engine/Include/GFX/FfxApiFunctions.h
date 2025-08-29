#pragma once
ZE_WARNING_PUSH
#include "ffx_api.h"
#include "ffx_api_types.h"
ZE_WARNING_POP

namespace ZE::GFX
{
	// Set of pointer for interacting with FFX API
	struct FfxApiFunctions
	{
		PfnFfxDestroyContext DestroyContext = nullptr;
		PfnFfxConfigure Configure = nullptr;
		PfnFfxQuery Query = nullptr;
		PfnFfxDispatch Dispatch = nullptr;
	};

	// Convert pixel format into FFX API surface format
	constexpr FfxApiSurfaceFormat GetFfxApiFormat(PixelFormat format) noexcept;
	// Convert FFX SDK surface format into pixel format
	constexpr PixelFormat GetPixelFormat(FfxApiSurfaceFormat format) noexcept;
	// Get resulting string for given FFX API error code
	constexpr const char* GetFfxApiReturnString(ffxReturnCode_t code) noexcept;

#pragma region Functions
	constexpr FfxApiSurfaceFormat GetFfxApiFormat(PixelFormat format) noexcept
	{
		switch (format)
		{
		default:
			ZE_FAIL("Format not yet supported by FidelityFX SDK!");
			[[fallthrough]];
		case PixelFormat::Unknown:
			return FFX_API_SURFACE_FORMAT_UNKNOWN;
		case PixelFormat::R32G32B32A32_Float:
			return FFX_API_SURFACE_FORMAT_R32G32B32A32_FLOAT;
		case PixelFormat::R32G32B32A32_UInt:
			return FFX_API_SURFACE_FORMAT_R32G32B32A32_UINT;
		case PixelFormat::R32G32B32A32_SInt:
			ZE_WARNING("FidelityFX SDK is not supporting plain R32G32B32A32_SInt so falling back to typeless version!");
			return FFX_API_SURFACE_FORMAT_R32G32B32A32_TYPELESS;
		case PixelFormat::R16G16B16A16_Float:
			return FFX_API_SURFACE_FORMAT_R16G16B16A16_FLOAT;
		case PixelFormat::R16G16B16A16_UInt:
		case PixelFormat::R16G16B16A16_SInt:
		case PixelFormat::R16G16B16A16_UNorm:
		case PixelFormat::R16G16B16A16_SNorm:
			ZE_WARNING("FidelityFX SDK is not supporting plain R16G16B16A16_* so falling back to typeless version!");
			return FFX_API_SURFACE_FORMAT_R16G16B16A16_TYPELESS;
		case PixelFormat::R8G8B8A8_UInt:
		case PixelFormat::R8G8B8A8_SInt:
			ZE_WARNING("FidelityFX SDK is not supporting plain R32G32B32A32_SInt so falling back to typeless version!");
			return FFX_API_SURFACE_FORMAT_R8G8B8A8_TYPELESS;
		case PixelFormat::R8G8B8A8_UNorm:
			return FFX_API_SURFACE_FORMAT_R8G8B8A8_UNORM;
		case PixelFormat::R8G8B8A8_UNorm_SRGB:
			return FFX_API_SURFACE_FORMAT_R8G8B8A8_SRGB;
		case PixelFormat::R8G8B8A8_SNorm:
			return FFX_API_SURFACE_FORMAT_R8G8B8A8_SNORM;
		case PixelFormat::B8G8R8A8_UNorm:
			return FFX_API_SURFACE_FORMAT_B8G8R8A8_UNORM;
		case PixelFormat::B8G8R8A8_UNorm_SRGB:
			return FFX_API_SURFACE_FORMAT_B8G8R8A8_SRGB;
		case PixelFormat::R32G32B32_Float:
			return FFX_API_SURFACE_FORMAT_R32G32B32_FLOAT;
		case PixelFormat::R32G32_Float:
			return FFX_API_SURFACE_FORMAT_R32G32_FLOAT;
		case PixelFormat::R32G32_UInt:
		case PixelFormat::R32G32_SInt:
			ZE_WARNING("FidelityFX SDK is not supporting plain R32G32_*Int so falling back to typeless version!");
			return FFX_API_SURFACE_FORMAT_R32G32_TYPELESS;
		case PixelFormat::R16G16_Float:
			return FFX_API_SURFACE_FORMAT_R16G16_FLOAT;
		case PixelFormat::R16G16_UInt:
			return FFX_API_SURFACE_FORMAT_R16G16_UINT;
		case PixelFormat::R16G16_SInt:
			return FFX_API_SURFACE_FORMAT_R16G16_SINT;
		case PixelFormat::R16G16_UNorm:
		case PixelFormat::R16G16_SNorm:
			ZE_WARNING("FidelityFX SDK is not supporting plain R16G16_*Norm so falling back to typeless version!");
			return FFX_API_SURFACE_FORMAT_R16G16_TYPELESS;
		case PixelFormat::R8G8_UInt:
			return FFX_API_SURFACE_FORMAT_R8G8_UINT;
		case PixelFormat::R8G8_UNorm:
			return FFX_API_SURFACE_FORMAT_R8G8_UNORM;
		case PixelFormat::R8G8_SInt:
		case PixelFormat::R8G8_SNorm:
			ZE_WARNING("FidelityFX SDK is not supporting plain R8G8_S* so falling back to typeless version!");
			return FFX_API_SURFACE_FORMAT_R8G8_TYPELESS;
		case PixelFormat::R32_Float:
		case PixelFormat::R32_Depth:
			return FFX_API_SURFACE_FORMAT_R32_FLOAT;
		case PixelFormat::R32_UInt:
			return FFX_API_SURFACE_FORMAT_R32_UINT;
		case PixelFormat::R32_SInt:
			ZE_WARNING("FidelityFX SDK is not supporting R32_SInt so falling back to typeless version!");
			return FFX_API_SURFACE_FORMAT_R32_TYPELESS;
		case PixelFormat::R16_Float:
		case PixelFormat::R16_Depth:
			return FFX_API_SURFACE_FORMAT_R16_FLOAT;
		case PixelFormat::R16_UInt:
			return FFX_API_SURFACE_FORMAT_R16_UINT;
		case PixelFormat::R16_SInt:
			ZE_WARNING("FidelityFX SDK is not supporting plain R16_SInt so falling back to typeless version!");
			return FFX_API_SURFACE_FORMAT_R16_TYPELESS;
		case PixelFormat::R16_UNorm:
			return FFX_API_SURFACE_FORMAT_R16_UNORM;
		case PixelFormat::R16_SNorm:
			return FFX_API_SURFACE_FORMAT_R16_SNORM;
		case PixelFormat::R8_UInt:
			return FFX_API_SURFACE_FORMAT_R8_UINT;
		case PixelFormat::R8_UNorm:
			return FFX_API_SURFACE_FORMAT_R8_UNORM;
		case PixelFormat::R8_SInt:
		case PixelFormat::R8_SNorm:
			ZE_WARNING("FidelityFX SDK is not supporting plain R8_S* so falling back to typeless version!");
			return FFX_API_SURFACE_FORMAT_R8_TYPELESS;
		case PixelFormat::R10G10B10A2_UInt:
			ZE_WARNING("FidelityFX SDK is not supporting plain R10G10B10A2_UInt so falling back to typeless version!");
			return FFX_API_SURFACE_FORMAT_R10G10B10A2_TYPELESS;
		case PixelFormat::R10G10B10A2_UNorm:
			return FFX_API_SURFACE_FORMAT_R10G10B10A2_UNORM;
		case PixelFormat::R11G11B10_Float:
			return FFX_API_SURFACE_FORMAT_R11G11B10_FLOAT;
		case PixelFormat::R9G9B9E5_SharedExp:
			return FFX_API_SURFACE_FORMAT_R9G9B9E5_SHAREDEXP;
		}
	}

	constexpr PixelFormat GetPixelFormat(FfxApiSurfaceFormat format) noexcept
	{
		switch (format)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case FFX_API_SURFACE_FORMAT_UNKNOWN:
			return PixelFormat::Unknown;
		case FFX_API_SURFACE_FORMAT_R32G32B32A32_TYPELESS:
			ZE_WARNING("Typeless format detected, falling back to corresponding R32G32B32A32_UInt!");
			[[fallthrough]];
		case FFX_API_SURFACE_FORMAT_R32G32B32A32_UINT:
			return PixelFormat::R32G32B32A32_UInt;
		case FFX_API_SURFACE_FORMAT_R32G32B32A32_FLOAT:
			return PixelFormat::R32G32B32A32_Float;
		case FFX_API_SURFACE_FORMAT_R16G16B16A16_TYPELESS:
			ZE_WARNING("Typeless format detected, falling back to corresponding R16G16B16A16_Float!");
			[[fallthrough]];
		case FFX_API_SURFACE_FORMAT_R16G16B16A16_FLOAT:
			return PixelFormat::R16G16B16A16_Float;
		case FFX_API_SURFACE_FORMAT_R32G32B32_FLOAT:
			return PixelFormat::R32G32B32_Float;
		case FFX_API_SURFACE_FORMAT_R32G32_TYPELESS:
			ZE_WARNING("Typeless format detected, falling back to corresponding R32G32_Float!");
			[[fallthrough]];
		case FFX_API_SURFACE_FORMAT_R32G32_FLOAT:
			return PixelFormat::R32G32_Float;
		case FFX_API_SURFACE_FORMAT_R8_UINT:
			return PixelFormat::R8_UInt;
		case FFX_API_SURFACE_FORMAT_R32_UINT:
			return PixelFormat::R32_UInt;
		case FFX_API_SURFACE_FORMAT_R8G8B8A8_TYPELESS:
			ZE_WARNING("Typeless format detected, falling back to corresponding R8G8B8A8_UInt!");
			return PixelFormat::R8G8B8A8_UInt;
		case FFX_API_SURFACE_FORMAT_R8G8B8A8_UNORM:
			return PixelFormat::R8G8B8A8_UNorm;
		case FFX_API_SURFACE_FORMAT_R8G8B8A8_SNORM:
			return PixelFormat::R8G8B8A8_SNorm;
		case FFX_API_SURFACE_FORMAT_R8G8B8A8_SRGB:
			return PixelFormat::R8G8B8A8_UNorm_SRGB;
		case FFX_API_SURFACE_FORMAT_B8G8R8A8_TYPELESS:
			ZE_WARNING("Typeless format detected, falling back to corresponding B8G8R8A8_UNorm!");
			[[fallthrough]];
		case FFX_API_SURFACE_FORMAT_B8G8R8A8_UNORM:
			return PixelFormat::B8G8R8A8_UNorm;
		case FFX_API_SURFACE_FORMAT_B8G8R8A8_SRGB:
			return PixelFormat::B8G8R8A8_UNorm_SRGB;
		case FFX_API_SURFACE_FORMAT_R11G11B10_FLOAT:
			return PixelFormat::R11G11B10_Float;
		case FFX_API_SURFACE_FORMAT_R10G10B10A2_TYPELESS:
			ZE_WARNING("Typeless format detected, falling back to corresponding R10G10B10A2_UNorm!");
			[[fallthrough]];
		case FFX_API_SURFACE_FORMAT_R10G10B10A2_UNORM:
			return PixelFormat::R10G10B10A2_UNorm;
		case FFX_API_SURFACE_FORMAT_R16G16_FLOAT:
			return PixelFormat::R16G16_Float;
		case FFX_API_SURFACE_FORMAT_R16G16_TYPELESS:
			ZE_WARNING("Typeless format detected, falling back to corresponding R16G16_UInt!");
			[[fallthrough]];
		case FFX_API_SURFACE_FORMAT_R16G16_UINT:
			return PixelFormat::R16G16_UInt;
		case FFX_API_SURFACE_FORMAT_R16G16_SINT:
			return PixelFormat::R16G16_SInt;
		case FFX_API_SURFACE_FORMAT_R16_FLOAT:
			return PixelFormat::R16_Float;
		case FFX_API_SURFACE_FORMAT_R16_TYPELESS:
			ZE_WARNING("Typeless format detected, falling back to corresponding R16_UInt!");
			[[fallthrough]];
		case FFX_API_SURFACE_FORMAT_R16_UINT:
			return PixelFormat::R16_UInt;
		case FFX_API_SURFACE_FORMAT_R16_UNORM:
			return PixelFormat::R16_UNorm;
		case FFX_API_SURFACE_FORMAT_R16_SNORM:
			return PixelFormat::R16_SNorm;
		case FFX_API_SURFACE_FORMAT_R8_TYPELESS:
			ZE_WARNING("Typeless format detected, falling back to corresponding R8_UNorm!");
			[[fallthrough]];
		case FFX_API_SURFACE_FORMAT_R8_UNORM:
			return PixelFormat::R8_UNorm;
		case FFX_API_SURFACE_FORMAT_R8G8_TYPELESS:
			ZE_WARNING("Typeless format detected, falling back to corresponding R8G8_UNorm!");
			[[fallthrough]];
		case FFX_API_SURFACE_FORMAT_R8G8_UNORM:
			return PixelFormat::R8G8_UNorm;
		case FFX_API_SURFACE_FORMAT_R8G8_UINT:
			return PixelFormat::R8G8_UInt;
		case FFX_API_SURFACE_FORMAT_R32_TYPELESS:
			ZE_WARNING("Typeless format detected, falling back to corresponding R32_FloatR32_Float!");
			[[fallthrough]];
		case FFX_API_SURFACE_FORMAT_R32_FLOAT:
			return PixelFormat::R32_Float;
		case FFX_API_SURFACE_FORMAT_R9G9B9E5_SHAREDEXP:
			return PixelFormat::R9G9B9E5_SharedExp;
		}
	}

	constexpr const char* GetFfxApiReturnString(ffxReturnCode_t code) noexcept
	{
#define DECODE(type, msg) case FFX_API_RETURN_##type: return #type ", " msg
		switch (code)
		{
			DECODE(OK, "the oparation was successful");
			DECODE(ERROR, "an error occurred that is not further specified");
			DECODE(ERROR_UNKNOWN_DESCTYPE, "the structure type given was not recognized for the function or context with which it was used, this is likely a programming error");
			DECODE(ERROR_RUNTIME_ERROR, "the underlying runtime (e.g. D3D12, Vulkan) or effect returned an error code");
			DECODE(NO_PROVIDER, "no provider was found for the given structure type, this is likely a programming error");
			DECODE(ERROR_MEMORY, "a memory allocation failed");
			DECODE(ERROR_PARAMETER, "a parameter was invalid, e.g. a null pointer, empty resource or out-of-bounds enum value");
		default:
			return "Unknown error code";
		}
#undef DECODE
	}
#pragma endregion
}

// Variable holding last result code of FFX API SDK
#define ZE_FFX_API_EXCEPT_RESULT __ffxApiReturnCode
// Enable usage of ZE_FFX_* macros in current scope
#define ZE_FFX_API_ENABLE() [[maybe_unused]] ffxReturnCode_t ZE_FFX_API_EXCEPT_RESULT
// Performs assert check on return value of FFX SDK call, before using needs call to ZE_FFX_ENABLE()
#define ZE_FFX_API_CHECK(call, info) ZE_FFX_API_EXCEPT_RESULT = (call); ZE_ASSERT(ZE_FFX_API_EXCEPT_RESULT == FFX_API_RETURN_OK, info)