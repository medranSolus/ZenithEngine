#pragma once
#if _ZE_FFX_API_ENABLED
ZE_WARNING_PUSH
#	include "ffx_api.h"
#	include "ffx_api_types.h"
ZE_WARNING_POP

namespace ZE::GFX::External
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

#	pragma region Functions
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
#	pragma endregion
}
#endif