#pragma once
#include "PixelFormat.h"

// To be changed into multiplatform library
#define XMVECTOR Vector
#include "WarningGuardOn.h"
#include "DirectXTex.h"
#include "WarningGuardOff.h"
#undef XMVECTOR
namespace ZE::Tex
{
	using namespace DirectX;
}

namespace ZE::GFX::API::DX
{
	constexpr DXGI_FORMAT GetDXFormat(PixelFormat format) noexcept;
	constexpr PixelFormat GetFormatFromDX(DXGI_FORMAT format) noexcept;
	constexpr DXGI_FORMAT ConvertFromDepthStencilFormat(DXGI_FORMAT format) noexcept;

#pragma region Functions
	constexpr DXGI_FORMAT GetDXFormat(PixelFormat format) noexcept
	{
		switch (format)
		{
		default:
		case PixelFormat::Unknown:						return DXGI_FORMAT_UNKNOWN;
		case PixelFormat::R32G32B32A32_Typeless:		return DXGI_FORMAT_R32G32B32A32_TYPELESS;
		case PixelFormat::R32G32B32A32_Float:			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case PixelFormat::R32G32B32A32_UInt:			return DXGI_FORMAT_R32G32B32A32_UINT;
		case PixelFormat::R32G32B32A32_SInt:			return DXGI_FORMAT_R32G32B32A32_SINT;
		case PixelFormat::R16G16B16A16_Typeless:		return DXGI_FORMAT_R16G16B16A16_TYPELESS;
		case PixelFormat::R16G16B16A16_Float:			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case PixelFormat::R16G16B16A16_UInt:			return DXGI_FORMAT_R16G16B16A16_UINT;
		case PixelFormat::R16G16B16A16_SInt:			return DXGI_FORMAT_R16G16B16A16_SINT;
		case PixelFormat::R16G16B16A16_UNorm:			return DXGI_FORMAT_R16G16B16A16_UNORM;
		case PixelFormat::R16G16B16A16_SNorm:			return DXGI_FORMAT_R16G16B16A16_SNORM;
		case PixelFormat::R8G8B8A8_Typeless:			return DXGI_FORMAT_R8G8B8A8_TYPELESS;
		case PixelFormat::R8G8B8A8_UInt:				return DXGI_FORMAT_R8G8B8A8_UINT;
		case PixelFormat::R8G8B8A8_SInt:				return DXGI_FORMAT_R8G8B8A8_SINT;
		case PixelFormat::R8G8B8A8_UNorm:				return DXGI_FORMAT_R8G8B8A8_UNORM;
		case PixelFormat::R8G8B8A8_UNorm_SRGB:			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		case PixelFormat::R8G8B8A8_SNorm:				return DXGI_FORMAT_R8G8B8A8_SNORM;
		case PixelFormat::B8G8R8A8_Typeless:			return DXGI_FORMAT_B8G8R8A8_TYPELESS;
		case PixelFormat::B8G8R8A8_UNorm:				return DXGI_FORMAT_B8G8R8A8_UNORM;
		case PixelFormat::B8G8R8A8_UNorm_SRGB:			return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		case PixelFormat::B8G8R8X8_Typeless:			return DXGI_FORMAT_B8G8R8X8_TYPELESS;
		case PixelFormat::B8G8R8X8_UNorm:				return DXGI_FORMAT_B8G8R8X8_UNORM;
		case PixelFormat::B8G8R8X8_UNorm_SRGB:			return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
		case PixelFormat::R32G32B32_Typeless:			return DXGI_FORMAT_R32G32B32_TYPELESS;
		case PixelFormat::R32G32B32_Float:				return DXGI_FORMAT_R32G32B32_FLOAT;
		case PixelFormat::R32G32B32_UInt:				return DXGI_FORMAT_R32G32B32_UINT;
		case PixelFormat::R32G32B32_SInt:				return DXGI_FORMAT_R32G32B32_SINT;
		case PixelFormat::R32G32_Typeless:				return DXGI_FORMAT_R32G32_TYPELESS;
		case PixelFormat::R32G32_Float:					return DXGI_FORMAT_R32G32_FLOAT;
		case PixelFormat::R32G32_UInt:					return DXGI_FORMAT_R32G32_UINT;
		case PixelFormat::R32G32_SInt:					return DXGI_FORMAT_R32G32_SINT;
		case PixelFormat::R16G16_Typeless:				return DXGI_FORMAT_R16G16_TYPELESS;
		case PixelFormat::R16G16_Float:					return DXGI_FORMAT_R16G16_FLOAT;
		case PixelFormat::R16G16_UInt:					return DXGI_FORMAT_R16G16_UINT;
		case PixelFormat::R16G16_SInt:					return DXGI_FORMAT_R16G16_SINT;
		case PixelFormat::R16G16_UNorm:					return DXGI_FORMAT_R16G16_UNORM;
		case PixelFormat::R16G16_SNorm:					return DXGI_FORMAT_R16G16_SNORM;
		case PixelFormat::R8G8_Typeless:				return DXGI_FORMAT_R8G8_TYPELESS;
		case PixelFormat::R8G8_UInt:					return DXGI_FORMAT_R8G8_UINT;
		case PixelFormat::R8G8_SInt:					return DXGI_FORMAT_R8G8_SINT;
		case PixelFormat::R8G8_UNorm:					return DXGI_FORMAT_R8G8_UNORM;
		case PixelFormat::R8G8_SNorm:					return DXGI_FORMAT_R8G8_SNORM;
		case PixelFormat::R32_Typeless:					return DXGI_FORMAT_R32_TYPELESS;
		case PixelFormat::R32_Float:					return DXGI_FORMAT_R32_FLOAT;
		case PixelFormat::R32_Depth:					return DXGI_FORMAT_D32_FLOAT;
		case PixelFormat::R32_UInt:						return DXGI_FORMAT_R32_UINT;
		case PixelFormat::R32_SInt:						return DXGI_FORMAT_R32_SINT;
		case PixelFormat::R16_Typeless:					return DXGI_FORMAT_R16_TYPELESS;
		case PixelFormat::R16_Float:					return DXGI_FORMAT_R16_FLOAT;
		case PixelFormat::R16_UInt:						return DXGI_FORMAT_R16_UINT;
		case PixelFormat::R16_SInt:						return DXGI_FORMAT_R16_SINT;
		case PixelFormat::R16_UNorm:					return DXGI_FORMAT_R16_UNORM;
		case PixelFormat::R16_SNorm:					return DXGI_FORMAT_R16_SNORM;
		case PixelFormat::R16_Depth:					return DXGI_FORMAT_D16_UNORM;
		case PixelFormat::R8_Typeless:					return DXGI_FORMAT_R8_TYPELESS;
		case PixelFormat::R8_UInt:						return DXGI_FORMAT_R8_UINT;
		case PixelFormat::R8_SInt:						return DXGI_FORMAT_R8_SINT;
		case PixelFormat::R8_UNorm:						return DXGI_FORMAT_R8_UNORM;
		case PixelFormat::R8_SNorm:						return DXGI_FORMAT_R8_SNORM;
		case PixelFormat::A8_UNorm:						return DXGI_FORMAT_A8_UNORM;
		case PixelFormat::BC1_Typeless:					return DXGI_FORMAT_BC1_TYPELESS;
		case PixelFormat::BC1_UNorm:					return DXGI_FORMAT_BC1_UNORM;
		case PixelFormat::BC1_UNorm_SRGB:				return DXGI_FORMAT_BC1_UNORM_SRGB;
		case PixelFormat::BC2_Typeless:					return DXGI_FORMAT_BC2_TYPELESS;
		case PixelFormat::BC2_UNorm:					return DXGI_FORMAT_BC2_UNORM;
		case PixelFormat::BC2_UNorm_SRGB:				return DXGI_FORMAT_BC2_UNORM_SRGB;
		case PixelFormat::BC3_Typeless:					return DXGI_FORMAT_BC3_TYPELESS;
		case PixelFormat::BC3_UNorm:					return DXGI_FORMAT_BC3_UNORM;
		case PixelFormat::BC3_UNorm_SRGB:				return DXGI_FORMAT_BC3_UNORM_SRGB;
		case PixelFormat::BC4_Typeless:					return DXGI_FORMAT_BC4_TYPELESS;
		case PixelFormat::BC4_UNorm:					return DXGI_FORMAT_BC4_UNORM;
		case PixelFormat::BC4_SNorm:					return DXGI_FORMAT_BC4_SNORM;
		case PixelFormat::BC5_Typeless:					return DXGI_FORMAT_BC5_TYPELESS;
		case PixelFormat::BC5_UNorm:					return DXGI_FORMAT_BC5_UNORM;
		case PixelFormat::BC5_SNorm:					return DXGI_FORMAT_BC5_SNORM;
		case PixelFormat::BC6H_Typeless:				return DXGI_FORMAT_BC6H_TYPELESS;
		case PixelFormat::BC6H_UF16:					return DXGI_FORMAT_BC6H_UF16;
		case PixelFormat::BC6H_SF16:					return DXGI_FORMAT_BC6H_SF16;
		case PixelFormat::BC7_Typeless:					return DXGI_FORMAT_BC7_TYPELESS;
		case PixelFormat::BC7_UNorm:					return DXGI_FORMAT_BC7_UNORM;
		case PixelFormat::BC7_UNorm_SRGB:				return DXGI_FORMAT_BC7_UNORM_SRGB;
		case PixelFormat::R24G8_Typeless:				return DXGI_FORMAT_R24G8_TYPELESS;
		case PixelFormat::R24_UNorm_X8_Typeless:		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		case PixelFormat::R24_Depth_S8_UInt:			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case PixelFormat::X24_Typeless_G8_UInt:			return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
		case PixelFormat::R32_Depth_S8X24_UInt:			return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		case PixelFormat::R32_Float_X8X24_Typeless:		return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		case PixelFormat::X32_Typeless_G8X24_UInt:		return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
		case PixelFormat::R10G10B10A2_Typeless:			return DXGI_FORMAT_R10G10B10A2_TYPELESS;
		case PixelFormat::R10G10B10A2_UInt:				return DXGI_FORMAT_R10G10B10A2_UINT;
		case PixelFormat::R10G10B10A2_UNorm:			return DXGI_FORMAT_R10G10B10A2_UNORM;
		case PixelFormat::B4G4R4A4_UNorm:				return DXGI_FORMAT_B4G4R4A4_UNORM;
		case PixelFormat::R8G8_B8G8_UNorm:				return DXGI_FORMAT_R8G8_B8G8_UNORM;
		case PixelFormat::G8R8_G8B8_UNorm:				return DXGI_FORMAT_G8R8_G8B8_UNORM;
		case PixelFormat::R10G10B10_XR_Bias_A2_UNorm:	return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
		case PixelFormat::R9G9B9E5_SharedExp:			return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
		case PixelFormat::R11G11B10_Float:				return DXGI_FORMAT_R11G11B10_FLOAT;
		case PixelFormat::B5G5R5A1_UNorm:				return DXGI_FORMAT_B5G5R5A1_UNORM;
		case PixelFormat::B5G6R5_UNorm:					return DXGI_FORMAT_B5G6R5_UNORM;
		case PixelFormat::R32G8X24_Typeless:			return DXGI_FORMAT_R32G8X24_TYPELESS;
		case PixelFormat::R1_UNorm:						return DXGI_FORMAT_R1_UNORM;
		}
	}

	constexpr PixelFormat GetFormatFromDX(DXGI_FORMAT format) noexcept
	{
		switch (format)
		{
		default:
		case DXGI_FORMAT_AYUV:
		case DXGI_FORMAT_Y410:
		case DXGI_FORMAT_Y416:
		case DXGI_FORMAT_NV12:
		case DXGI_FORMAT_P010:
		case DXGI_FORMAT_P016:
		case DXGI_FORMAT_420_OPAQUE:
		case DXGI_FORMAT_YUY2:
		case DXGI_FORMAT_Y210:
		case DXGI_FORMAT_Y216:
		case DXGI_FORMAT_NV11:
		case DXGI_FORMAT_AI44:
		case DXGI_FORMAT_IA44:
		case DXGI_FORMAT_P8:
		case DXGI_FORMAT_A8P8:
		case DXGI_FORMAT_P208:
		case DXGI_FORMAT_V208:
		case DXGI_FORMAT_V408:
		case DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE:
		case DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE:
		case DXGI_FORMAT_FORCE_UINT:
			assert(false && "Trying to convert unsupported format!");
		case DXGI_FORMAT_UNKNOWN:						return PixelFormat::Unknown;
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:			return PixelFormat::R32G32B32A32_Typeless;
		case DXGI_FORMAT_R32G32B32A32_FLOAT:			return PixelFormat::R32G32B32A32_Float;
		case DXGI_FORMAT_R32G32B32A32_UINT:				return PixelFormat::R32G32B32A32_UInt;
		case DXGI_FORMAT_R32G32B32A32_SINT:				return PixelFormat::R32G32B32A32_SInt;
		case DXGI_FORMAT_R32G32B32_TYPELESS:			return PixelFormat::R32G32B32_Typeless;
		case DXGI_FORMAT_R32G32B32_FLOAT:				return PixelFormat::R32G32B32_Float;
		case DXGI_FORMAT_R32G32B32_UINT:				return PixelFormat::R32G32B32_UInt;
		case DXGI_FORMAT_R32G32B32_SINT:				return PixelFormat::R32G32B32_SInt;
		case DXGI_FORMAT_R16G16B16A16_TYPELESS:			return PixelFormat::R16G16B16A16_Typeless;
		case DXGI_FORMAT_R16G16B16A16_FLOAT:			return PixelFormat::R16G16B16A16_Float;
		case DXGI_FORMAT_R16G16B16A16_UNORM:			return PixelFormat::R16G16B16A16_UNorm;
		case DXGI_FORMAT_R16G16B16A16_UINT:				return PixelFormat::R16G16B16A16_UInt;
		case DXGI_FORMAT_R16G16B16A16_SNORM:			return PixelFormat::R16G16B16A16_SNorm;
		case DXGI_FORMAT_R16G16B16A16_SINT:				return PixelFormat::R16G16B16A16_SInt;
		case DXGI_FORMAT_R32G32_TYPELESS:				return PixelFormat::R32G32_Typeless;
		case DXGI_FORMAT_R32G32_FLOAT:					return PixelFormat::R32G32_Float;
		case DXGI_FORMAT_R32G32_UINT:					return PixelFormat::R32G32_UInt;
		case DXGI_FORMAT_R32G32_SINT:					return PixelFormat::R32G32_SInt;
		case DXGI_FORMAT_R32G8X24_TYPELESS:				return PixelFormat::R32G8X24_Typeless;
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:			return PixelFormat::R32_Depth_S8X24_UInt;
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:		return PixelFormat::R32_Float_X8X24_Typeless;
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:		return PixelFormat::X32_Typeless_G8X24_UInt;
		case DXGI_FORMAT_R10G10B10A2_TYPELESS:			return PixelFormat::R10G10B10A2_Typeless;
		case DXGI_FORMAT_R10G10B10A2_UNORM:				return PixelFormat::R10G10B10A2_UNorm;
		case DXGI_FORMAT_R10G10B10A2_UINT:				return PixelFormat::R10G10B10A2_UInt;
		case DXGI_FORMAT_R11G11B10_FLOAT:				return PixelFormat::R11G11B10_Float;
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:				return PixelFormat::R8G8B8A8_Typeless;
		case DXGI_FORMAT_R8G8B8A8_UNORM:				return PixelFormat::R8G8B8A8_UNorm;
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:			return PixelFormat::R8G8B8A8_UNorm_SRGB;
		case DXGI_FORMAT_R8G8B8A8_UINT:					return PixelFormat::R8G8B8A8_UInt;
		case DXGI_FORMAT_R8G8B8A8_SNORM:				return PixelFormat::R8G8B8A8_SNorm;
		case DXGI_FORMAT_R8G8B8A8_SINT:					return PixelFormat::R8G8B8A8_SInt;
		case DXGI_FORMAT_R16G16_TYPELESS:				return PixelFormat::R16G16_Typeless;
		case DXGI_FORMAT_R16G16_FLOAT:					return PixelFormat::R16G16_Float;
		case DXGI_FORMAT_R16G16_UNORM:					return PixelFormat::R16G16_UNorm;
		case DXGI_FORMAT_R16G16_UINT:					return PixelFormat::R16G16_UInt;
		case DXGI_FORMAT_R16G16_SNORM:					return PixelFormat::R16G16_SNorm;
		case DXGI_FORMAT_R16G16_SINT:					return PixelFormat::R16G16_SInt;
		case DXGI_FORMAT_R32_TYPELESS:					return PixelFormat::R32_Typeless;
		case DXGI_FORMAT_D32_FLOAT:						return PixelFormat::R32_Depth;
		case DXGI_FORMAT_R32_FLOAT:						return PixelFormat::R32_Float;
		case DXGI_FORMAT_R32_UINT:						return PixelFormat::R32_UInt;
		case DXGI_FORMAT_R32_SINT:						return PixelFormat::R32_SInt;
		case DXGI_FORMAT_R24G8_TYPELESS:				return PixelFormat::R24G8_Typeless;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:				return PixelFormat::R24_Depth_S8_UInt;
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:			return PixelFormat::R24_UNorm_X8_Typeless;
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:			return PixelFormat::X24_Typeless_G8_UInt;
		case DXGI_FORMAT_R8G8_TYPELESS:					return PixelFormat::R8G8_Typeless;
		case DXGI_FORMAT_R8G8_UNORM:					return PixelFormat::R8G8_UNorm;
		case DXGI_FORMAT_R8G8_UINT:						return PixelFormat::R8G8_UInt;
		case DXGI_FORMAT_R8G8_SNORM:					return PixelFormat::R8G8_SNorm;
		case DXGI_FORMAT_R8G8_SINT:						return PixelFormat::R8G8_SInt;
		case DXGI_FORMAT_R16_TYPELESS:					return PixelFormat::R16_Typeless;
		case DXGI_FORMAT_R16_FLOAT:						return PixelFormat::R16_Float;
		case DXGI_FORMAT_D16_UNORM:						return PixelFormat::R16_Depth;
		case DXGI_FORMAT_R16_UNORM:						return PixelFormat::R16_UNorm;
		case DXGI_FORMAT_R16_UINT:						return PixelFormat::R16_UInt;
		case DXGI_FORMAT_R16_SNORM:						return PixelFormat::R16_SNorm;
		case DXGI_FORMAT_R16_SINT:						return PixelFormat::R16_SInt;
		case DXGI_FORMAT_R8_TYPELESS:					return PixelFormat::R8_Typeless;
		case DXGI_FORMAT_R8_UNORM:						return PixelFormat::R8_UNorm;
		case DXGI_FORMAT_R8_UINT:						return PixelFormat::R8_UInt;
		case DXGI_FORMAT_R8_SNORM:						return PixelFormat::R8_SNorm;
		case DXGI_FORMAT_R8_SINT:						return PixelFormat::R8_SInt;
		case DXGI_FORMAT_A8_UNORM:						return PixelFormat::A8_UNorm;
		case DXGI_FORMAT_R1_UNORM:						return PixelFormat::R1_UNorm;
		case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:			return PixelFormat::R9G9B9E5_SharedExp;
		case DXGI_FORMAT_R8G8_B8G8_UNORM:				return PixelFormat::R8G8_B8G8_UNorm;
		case DXGI_FORMAT_G8R8_G8B8_UNORM:				return PixelFormat::G8R8_G8B8_UNorm;
		case DXGI_FORMAT_BC1_TYPELESS:					return PixelFormat::BC1_Typeless;
		case DXGI_FORMAT_BC1_UNORM:						return PixelFormat::BC1_UNorm;
		case DXGI_FORMAT_BC1_UNORM_SRGB:				return PixelFormat::BC1_UNorm_SRGB;
		case DXGI_FORMAT_BC2_TYPELESS:					return PixelFormat::BC2_Typeless;
		case DXGI_FORMAT_BC2_UNORM:						return PixelFormat::BC2_UNorm;
		case DXGI_FORMAT_BC2_UNORM_SRGB:				return PixelFormat::BC2_UNorm_SRGB;
		case DXGI_FORMAT_BC3_TYPELESS:					return PixelFormat::BC3_Typeless;
		case DXGI_FORMAT_BC3_UNORM:						return PixelFormat::BC3_UNorm;
		case DXGI_FORMAT_BC3_UNORM_SRGB:				return PixelFormat::BC3_UNorm_SRGB;
		case DXGI_FORMAT_BC4_TYPELESS:					return PixelFormat::BC4_Typeless;
		case DXGI_FORMAT_BC4_UNORM:						return PixelFormat::BC4_UNorm;
		case DXGI_FORMAT_BC4_SNORM:						return PixelFormat::BC4_SNorm;
		case DXGI_FORMAT_BC5_TYPELESS:					return PixelFormat::BC5_Typeless;
		case DXGI_FORMAT_BC5_UNORM:						return PixelFormat::BC5_UNorm;
		case DXGI_FORMAT_BC5_SNORM:						return PixelFormat::BC5_SNorm;
		case DXGI_FORMAT_B5G6R5_UNORM:					return PixelFormat::B5G6R5_UNorm;
		case DXGI_FORMAT_B5G5R5A1_UNORM:				return PixelFormat::B5G5R5A1_UNorm;
		case DXGI_FORMAT_B8G8R8A8_UNORM:				return PixelFormat::B8G8R8A8_UNorm;
		case DXGI_FORMAT_B8G8R8X8_UNORM:				return PixelFormat::B8G8R8X8_UNorm;
		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:	return PixelFormat::R10G10B10_XR_Bias_A2_UNorm;
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:				return PixelFormat::B8G8R8A8_Typeless;
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:			return PixelFormat::B8G8R8A8_UNorm_SRGB;
		case DXGI_FORMAT_B8G8R8X8_TYPELESS:				return PixelFormat::B8G8R8X8_Typeless;
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:			return PixelFormat::B8G8R8X8_UNorm_SRGB;
		case DXGI_FORMAT_BC6H_TYPELESS:					return PixelFormat::BC6H_Typeless;
		case DXGI_FORMAT_BC6H_UF16:						return PixelFormat::BC6H_UF16;
		case DXGI_FORMAT_BC6H_SF16:						return PixelFormat::BC6H_SF16;
		case DXGI_FORMAT_BC7_TYPELESS:					return PixelFormat::BC7_Typeless;
		case DXGI_FORMAT_BC7_UNORM:						return PixelFormat::BC7_UNorm;
		case DXGI_FORMAT_BC7_UNORM_SRGB:				return PixelFormat::BC7_UNorm_SRGB;
		case DXGI_FORMAT_B4G4R4A4_UNORM:				return PixelFormat::B4G4R4A4_UNorm;
		}
	}

	constexpr DXGI_FORMAT ConvertFromDepthStencilFormat(DXGI_FORMAT format) noexcept
	{
		switch (format)
		{
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_D32_FLOAT:
			return DXGI_FORMAT_R32_FLOAT;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		case DXGI_FORMAT_D16_UNORM:
			return DXGI_FORMAT_R16_UNORM;
		default:
			return format;
		}
	}
#pragma endregion
}