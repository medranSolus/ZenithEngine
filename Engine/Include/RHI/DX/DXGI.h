#pragma once
// Headers needed for DirectX Graphics Infrastructure
#include "GFX/Resource/Topology.h"
#include "DebugInfoManager.h"
ZE_WARNING_PUSH
#include <dxgi1_6.h>
#include <d3dcompiler.h>
ZE_WARNING_POP

namespace ZE::RHI::DX
{
	// Wrappers for DXGI interfaces (rest is in DebugInfoManager.h)
	typedef IDXGIAdapter4   IAdapter;
	typedef IDXGIDevice4    IDevice;
	typedef IDXGIFactory7   IFactory;
	typedef IDXGISwapChain4 ISwapChain;

	// Convert PixelFormat to DXGI_FORMAT
	constexpr DXGI_FORMAT GetDXFormat(PixelFormat format) noexcept;
	// Convert DXGI_FORMAT to PixelFormat
	constexpr PixelFormat GetFormatFromDX(DXGI_FORMAT format) noexcept;
	// Convert PixelFormat to DXGI_FORMAT with only formats suitable for views over depth stencil
	constexpr DXGI_FORMAT GetNonDepthDXFormat(PixelFormat format) noexcept;
	// Convert PixelFormat to DXGI_FORMAT with formats suitable for creation of DSV
	constexpr DXGI_FORMAT GetTypedDepthDXFormat(PixelFormat format) noexcept;
	// Convert current format to be suitable for view over depth stencil in shaders
	constexpr DXGI_FORMAT ConvertDepthFormatToResourceView(DXGI_FORMAT format, bool useStencil) noexcept;
	// Convert current format to be suitable for creation of DSV
	constexpr DXGI_FORMAT ConvertDepthFormatToDSV(DXGI_FORMAT format) noexcept;
	// Check whether depth stencil format supports only depth
	constexpr bool IsDepthOnly(DXGI_FORMAT format) noexcept;

	// Creates DXGI Factory
	ComPtr<IFactory> CreateFactory(
#if _ZE_DEBUG_GFX_API
		DebugInfoManager& debugManager
#endif
	);

	// Creates DXGI Factory and enumerates available GPU adapters in order of highest performant
	ComPtr<IAdapter> CreateAdapter(
#if _ZE_DEBUG_GFX_API
		DebugInfoManager& debugManager
#endif
	);

	// Creates swap chain for window and returns present flags
	UINT CreateSwapChain(ComPtr<IFactory> factory, IUnknown* device,
		HWND window, ComPtr<ISwapChain>& swapChain, bool shaderInput
#if _ZE_DEBUG_GFX_API
		, DebugInfoManager& debugManager
#endif
	);

	// Get qualified primitive topology
	constexpr D3D_PRIMITIVE_TOPOLOGY GetTopology(GFX::Resource::TopologyType type, GFX::Resource::TopologyOrder order) noexcept;

#pragma region Functions
	// List of mappings between PixelFormat and DXGI_FORMAT for enum decoding in X() macro
#define ZE_DX_FORMAT_MAPPINGS \
	X(Unknown,             DXGI_FORMAT_UNKNOWN) \
	X(R32G32B32A32_Float,  DXGI_FORMAT_R32G32B32A32_FLOAT) \
	X(R32G32B32A32_UInt,   DXGI_FORMAT_R32G32B32A32_UINT) \
	X(R32G32B32A32_SInt,   DXGI_FORMAT_R32G32B32A32_SINT) \
	X(R16G16B16A16_Float,  DXGI_FORMAT_R16G16B16A16_FLOAT) \
	X(R16G16B16A16_UInt,   DXGI_FORMAT_R16G16B16A16_UINT) \
	X(R16G16B16A16_SInt,   DXGI_FORMAT_R16G16B16A16_SINT) \
	X(R16G16B16A16_UNorm,  DXGI_FORMAT_R16G16B16A16_UNORM) \
	X(R16G16B16A16_SNorm,  DXGI_FORMAT_R16G16B16A16_SNORM) \
	X(R8G8B8A8_UInt,       DXGI_FORMAT_R8G8B8A8_UINT) \
	X(R8G8B8A8_SInt,       DXGI_FORMAT_R8G8B8A8_SINT) \
	X(R8G8B8A8_UNorm,      DXGI_FORMAT_R8G8B8A8_UNORM) \
	X(R8G8B8A8_UNorm_SRGB, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) \
	X(R8G8B8A8_SNorm,      DXGI_FORMAT_R8G8B8A8_SNORM) \
	X(B8G8R8A8_UNorm,      DXGI_FORMAT_B8G8R8A8_UNORM) \
	X(B8G8R8A8_UNorm_SRGB, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB) \
	X(R32G32B32_Float,     DXGI_FORMAT_R32G32B32_FLOAT) \
	X(R32G32B32_UInt,      DXGI_FORMAT_R32G32B32_UINT) \
	X(R32G32B32_SInt,      DXGI_FORMAT_R32G32B32_SINT) \
	X(R32G32_Float,        DXGI_FORMAT_R32G32_FLOAT) \
	X(R32G32_UInt,         DXGI_FORMAT_R32G32_UINT) \
	X(R32G32_SInt,         DXGI_FORMAT_R32G32_SINT) \
	X(R16G16_Float,        DXGI_FORMAT_R16G16_FLOAT) \
	X(R16G16_UInt,         DXGI_FORMAT_R16G16_UINT) \
	X(R16G16_SInt,         DXGI_FORMAT_R16G16_SINT) \
	X(R16G16_UNorm,        DXGI_FORMAT_R16G16_UNORM) \
	X(R16G16_SNorm,        DXGI_FORMAT_R16G16_SNORM) \
	X(R8G8_UInt,           DXGI_FORMAT_R8G8_UINT) \
	X(R8G8_SInt,           DXGI_FORMAT_R8G8_SINT) \
	X(R8G8_UNorm,          DXGI_FORMAT_R8G8_UNORM) \
	X(R8G8_SNorm,          DXGI_FORMAT_R8G8_SNORM) \
	X(R32_Float,           DXGI_FORMAT_R32_FLOAT) \
	X(R32_Depth,           DXGI_FORMAT_D32_FLOAT) \
	X(R32_UInt,            DXGI_FORMAT_R32_UINT) \
	X(R32_SInt,            DXGI_FORMAT_R32_SINT) \
	X(R16_Float,           DXGI_FORMAT_R16_FLOAT) \
	X(R16_UInt,            DXGI_FORMAT_R16_UINT) \
	X(R16_SInt,            DXGI_FORMAT_R16_SINT) \
	X(R16_UNorm,           DXGI_FORMAT_R16_UNORM) \
	X(R16_SNorm,           DXGI_FORMAT_R16_SNORM) \
	X(R16_Depth,           DXGI_FORMAT_D16_UNORM) \
	X(R8_UInt,             DXGI_FORMAT_R8_UINT) \
	X(R8_SInt,             DXGI_FORMAT_R8_SINT) \
	X(R8_UNorm,            DXGI_FORMAT_R8_UNORM) \
	X(R8_SNorm,            DXGI_FORMAT_R8_SNORM) \
	X(R24G8_DepthStencil,  DXGI_FORMAT_R24G8_TYPELESS) \
	X(R32G8_DepthStencil,  DXGI_FORMAT_R32G8X24_TYPELESS) \
	X(R10G10B10A2_UInt,    DXGI_FORMAT_R10G10B10A2_UINT) \
	X(R10G10B10A2_UNorm,   DXGI_FORMAT_R10G10B10A2_UNORM) \
	X(R11G11B10_Float,     DXGI_FORMAT_R11G11B10_FLOAT) \
	X(R9G9B9E5_SharedExp,  DXGI_FORMAT_R9G9B9E5_SHAREDEXP) \
	X(B4G4R4A4_UNorm,      DXGI_FORMAT_B4G4R4A4_UNORM) \
	X(B5G5R5A1_UNorm,      DXGI_FORMAT_B5G5R5A1_UNORM) \
	X(B5G6R5_UNorm,        DXGI_FORMAT_B5G6R5_UNORM) \
	X(BC1_UNorm,           DXGI_FORMAT_BC1_UNORM) \
	X(BC1_UNorm_SRGB,      DXGI_FORMAT_BC1_UNORM_SRGB) \
	X(BC2_UNorm,           DXGI_FORMAT_BC2_UNORM) \
	X(BC2_UNorm_SRGB,      DXGI_FORMAT_BC2_UNORM_SRGB) \
	X(BC3_UNorm,           DXGI_FORMAT_BC3_UNORM) \
	X(BC3_UNorm_SRGB,      DXGI_FORMAT_BC3_UNORM_SRGB) \
	X(BC4_UNorm,           DXGI_FORMAT_BC4_UNORM) \
	X(BC4_SNorm,           DXGI_FORMAT_BC4_SNORM) \
	X(BC5_UNorm,           DXGI_FORMAT_BC5_UNORM) \
	X(BC5_SNorm,           DXGI_FORMAT_BC5_SNORM) \
	X(BC6H_UF16,           DXGI_FORMAT_BC6H_UF16) \
	X(BC6H_SF16,           DXGI_FORMAT_BC6H_SF16) \
	X(BC7_UNorm,           DXGI_FORMAT_BC7_UNORM) \
	X(BC7_UNorm_SRGB,      DXGI_FORMAT_BC7_UNORM_SRGB) \
	X(YUV_Y410,            DXGI_FORMAT_Y410) \
	X(YUV_Y216,            DXGI_FORMAT_Y216) \
	X(YUV_Y210,            DXGI_FORMAT_Y210) \
	X(YUV_YUY2,            DXGI_FORMAT_YUY2) \
	X(YUV_P208,            DXGI_FORMAT_P208) \
	X(YUV_P016,            DXGI_FORMAT_P016) \
	X(YUV_P010,            DXGI_FORMAT_P010) \
	X(YUV_NV12,            DXGI_FORMAT_NV12)

	constexpr DXGI_FORMAT GetDXFormat(PixelFormat format) noexcept
	{
#define X(pixelFormat, dxgiFormat) case PixelFormat::##pixelFormat: return dxgiFormat;
		switch (format)
		{
		default:
			ZE_ENUM_UNHANDLED();
			ZE_DX_FORMAT_MAPPINGS
		}
#undef X
	}

	constexpr PixelFormat GetFormatFromDX(DXGI_FORMAT format) noexcept
	{
#define X(pixelFormat, dxgiFormat) case dxgiFormat: return PixelFormat::##pixelFormat;
		switch (format)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case DXGI_FORMAT_A8_UNORM:
		case DXGI_FORMAT_R1_UNORM:
		case DXGI_FORMAT_R8G8_B8G8_UNORM:
		case DXGI_FORMAT_G8R8_G8B8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		case DXGI_FORMAT_420_OPAQUE:
		case DXGI_FORMAT_NV11:
		case DXGI_FORMAT_AI44:
		case DXGI_FORMAT_IA44:
		case DXGI_FORMAT_P8:
		case DXGI_FORMAT_A8P8:
		case DXGI_FORMAT_V208:
		case DXGI_FORMAT_V408:
		case DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE:
		case DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE:
		case DXGI_FORMAT_FORCE_UINT:
			// Temporalily disabled till find corresponding format in Vulkan
		case DXGI_FORMAT_AYUV:
		case DXGI_FORMAT_Y416:
			// Typeless formats won't be supported as it's concept not present in other APIs
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		case DXGI_FORMAT_R32G32B32_TYPELESS:
		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		case DXGI_FORMAT_R32G32_TYPELESS:
		case DXGI_FORMAT_R10G10B10A2_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R16G16_TYPELESS:
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_R8G8_TYPELESS:
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_R8_TYPELESS:
		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC7_TYPELESS:
			ZE_FAIL("Trying to convert unsupported format!");
			ZE_DX_FORMAT_MAPPINGS
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return PixelFormat::R32G8_DepthStencil;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			return PixelFormat::R24G8_DepthStencil;
		}
#undef X
	}

	constexpr DXGI_FORMAT GetNonDepthDXFormat(PixelFormat format) noexcept
	{
		switch (format)
		{
		case PixelFormat::R32_Depth:
			return DXGI_FORMAT_R32_TYPELESS;
		case PixelFormat::R16_Depth:
			return DXGI_FORMAT_R16_TYPELESS;
		default:
			return GetDXFormat(format);
		}
	}

	constexpr DXGI_FORMAT GetTypedDepthDXFormat(PixelFormat format) noexcept
	{
		switch (format)
		{
		case PixelFormat::R32G8_DepthStencil:
			return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		case PixelFormat::R24G8_DepthStencil:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		default:
			return GetDXFormat(format);
		}
	}

	constexpr DXGI_FORMAT ConvertDepthFormatToResourceView(DXGI_FORMAT format, bool useStencil) noexcept
	{
		switch (format)
		{
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
			return DXGI_FORMAT_R32_FLOAT;
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
			return DXGI_FORMAT_R16_UNORM;
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
			return useStencil ? DXGI_FORMAT_X24_TYPELESS_G8_UINT : DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
			return useStencil ? DXGI_FORMAT_X32_TYPELESS_G8X24_UINT : DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		default:
			return format;
		}
	}

	constexpr DXGI_FORMAT ConvertDepthFormatToDSV(DXGI_FORMAT format) noexcept
	{
		switch (format)
		{
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
			return DXGI_FORMAT_D32_FLOAT;
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
			return DXGI_FORMAT_D16_UNORM;
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
			return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		default:
		{
			ZE_FAIL("Format not supported for Depth Stencil!");
			return DXGI_FORMAT_UNKNOWN;
		}
		}
	}

	constexpr bool IsDepthOnly(DXGI_FORMAT format) noexcept
	{
		switch (format)
		{
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_D16_UNORM:
			return true;
		default:
			return false;
		}
	}

	constexpr D3D_PRIMITIVE_TOPOLOGY GetTopology(GFX::Resource::TopologyType type, GFX::Resource::TopologyOrder order) noexcept
	{
		switch (type)
		{
		case GFX::Resource::TopologyType::Point:
		{
			switch (order)
			{
			case GFX::Resource::TopologyOrder::List:
				return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
			default:
				ZE_FAIL("Wrong combination of TopologyType and TopologyOrder!");
			}
			break;
		}
		case GFX::Resource::TopologyType::Line:
		{
			switch (order)
			{
			case GFX::Resource::TopologyOrder::List:
				return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
			case GFX::Resource::TopologyOrder::Strip:
				return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
			case GFX::Resource::TopologyOrder::ListAdjacency:
				return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
			case GFX::Resource::TopologyOrder::StripAdjacency:
				return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
			default:
				ZE_FAIL("Wrong combination of TopologyType and TopologyOrder!");
			}
			break;
		}
		case GFX::Resource::TopologyType::Triangle:
		{
			switch (order)
			{
			case GFX::Resource::TopologyOrder::List:
				return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			case GFX::Resource::TopologyOrder::Strip:
				return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			case GFX::Resource::TopologyOrder::ListAdjacency:
				return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
			case GFX::Resource::TopologyOrder::StripAdjacency:
				return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
			default:
				ZE_FAIL("Wrong combination of TopologyType and TopologyOrder!");
			}
			break;
		}
		case GFX::Resource::TopologyType::ControlPoint:
		{
			switch (order)
			{
			case GFX::Resource::TopologyOrder::PatchList1:
				return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList2:
				return D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList3:
				return D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList4:
				return D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList5:
				return D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList6:
				return D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList7:
				return D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList8:
				return D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList9:
				return D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList10:
				return D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList11:
				return D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList12:
				return D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList13:
				return D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList14:
				return D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList15:
				return D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList16:
				return D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList17:
				return D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList18:
				return D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList19:
				return D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList20:
				return D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList21:
				return D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList22:
				return D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList23:
				return D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList24:
				return D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList25:
				return D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList26:
				return D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList27:
				return D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList28:
				return D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList29:
				return D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList30:
				return D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList31:
				return D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList32:
				return D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST;
			default:
				ZE_FAIL("Wrong combination of TopologyType and TopologyOrder!");
			}
			break;
		}
		default:
			break;
		}
		return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}
#pragma endregion
}