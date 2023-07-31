#pragma once
#include "GFX/Resource/PipelineStateDesc.h"
#include "GFX/Resource/SamplerDesc.h"
#include "GFX/ShaderPresence.h"
#include "Window/MainWindow.h"
#include "VulkanExtensions.h"

namespace ZE::RHI::VK
{
	// Create surface over current display buffer
	VkSurfaceKHR CreateSurface(const Window::MainWindow& window, VkInstance instance);
	// Get Vulkan version of border color for immutable sampler
	constexpr VkBorderColor GetBorderColor(GFX::Resource::Texture::EdgeColor color) noexcept;
	// Get Vulkan version of comparison function
	constexpr VkCompareOp GetComparisonFunc(GFX::Resource::CompareMethod func) noexcept;
	// Get Vulkan version of minification sampler filter
	constexpr VkFilter GetFilterTypeMin(GFX::Resource::SamplerFilter samplerType) noexcept;
	// Get Vulkan version of magnification sampler filter
	constexpr VkFilter GetFilterTypeMag(GFX::Resource::SamplerFilter samplerType) noexcept;
	// Get Vulkan version of mipmap sampler filter
	constexpr VkSamplerMipmapMode GetFilterTypeMip(GFX::Resource::SamplerFilter samplerType) noexcept;
	// Checks whether given sampler should be anisotropic
	constexpr bool GetFilterTypeIsAnisotropic(GFX::Resource::SamplerFilter samplerType) noexcept;
	// Get opposite of cull mode
	constexpr VkStencilFaceFlags GetStencilFace(GFX::Resource::CullMode mode) noexcept;
	// Get Vulkan version of culling modes
	constexpr VkCullModeFlags GetCulling(GFX::Resource::CullMode mode) noexcept;
	// Get Vulkan version of primitive topology
	constexpr VkPrimitiveTopology GetPrimitiveTopology(GFX::Resource::TopologyType type, GFX::Resource::TopologyOrder order) noexcept;
	// Get shader stage from shader type
	constexpr VkShaderStageFlags GetShaderStage(GFX::Resource::ShaderTypes type) noexcept;
	// Get Vulkan version of texture addressing mode
	constexpr VkSamplerAddressMode GetTextureAddressMode(GFX::Resource::Texture::AddressMode mode) noexcept;
	// Get Vulkan version of main primitive topology types
	constexpr VkPrimitiveTopology GetTopologyType(GFX::Resource::TopologyType type) noexcept;
	// Get number of elements in control patch list
	constexpr U32 GetPatchCount(GFX::Resource::TopologyOrder order) noexcept;
	// Convert PixelFormat to VkFormat
	constexpr VkFormat GetVkFormat(PixelFormat format) noexcept;
	// Convert VkFormat to PixelFormat
	constexpr PixelFormat GetFormatFromVk(VkFormat format) noexcept;

#pragma region Functions
	constexpr VkBorderColor GetBorderColor(GFX::Resource::Texture::EdgeColor color) noexcept
	{
		switch (color)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::Texture::EdgeColor::TransparentBlack:
			return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		case GFX::Resource::Texture::EdgeColor::SolidBlack:
			return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		case GFX::Resource::Texture::EdgeColor::SolidWhite:
			return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		}
	}

	constexpr VkCompareOp GetComparisonFunc(GFX::Resource::CompareMethod func) noexcept
	{
		switch (func)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::CompareMethod::Never:
			return VK_COMPARE_OP_NEVER;
		case GFX::Resource::CompareMethod::Less:
			return VK_COMPARE_OP_LESS;
		case GFX::Resource::CompareMethod::Equal:
			return VK_COMPARE_OP_EQUAL;
		case GFX::Resource::CompareMethod::LessEqual:
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		case GFX::Resource::CompareMethod::Greater:
			return VK_COMPARE_OP_GREATER;
		case GFX::Resource::CompareMethod::NotEqual:
			return VK_COMPARE_OP_NOT_EQUAL;
		case GFX::Resource::CompareMethod::GreaterEqual:
			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case GFX::Resource::CompareMethod::Always:
			return VK_COMPARE_OP_ALWAYS;
		}
	}

	constexpr VkFilter GetFilterTypeMin(GFX::Resource::SamplerFilter samplerType) noexcept
	{
		return samplerType & GFX::Resource::SamplerType::LinearMinification ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
	}

	constexpr VkFilter GetFilterTypeMag(GFX::Resource::SamplerFilter samplerType) noexcept
	{
		return samplerType & GFX::Resource::SamplerType::LinearMagnification ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
	}

	constexpr VkSamplerMipmapMode GetFilterTypeMip(GFX::Resource::SamplerFilter samplerType) noexcept
	{
		return samplerType & GFX::Resource::SamplerType::LinearMipSampling ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
	}

	constexpr bool GetFilterTypeIsAnisotropic(GFX::Resource::SamplerFilter samplerType) noexcept
	{
		return (samplerType & GFX::Resource::SamplerType::Anisotropic) == GFX::Resource::SamplerType::Anisotropic;
	}

	constexpr VkStencilFaceFlags GetStencilFace(GFX::Resource::CullMode mode) noexcept
	{
		switch (mode)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::CullMode::None:
			return VK_STENCIL_FACE_FRONT_AND_BACK;
		case GFX::Resource::CullMode::Front:
			return VK_STENCIL_FACE_BACK_BIT;
		case GFX::Resource::CullMode::Back:
			return VK_STENCIL_FACE_FRONT_BIT;
		}
	}

	constexpr VkCullModeFlags GetCulling(GFX::Resource::CullMode mode) noexcept
	{
		switch (mode)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::CullMode::None:
			return VK_CULL_MODE_NONE;
		case GFX::Resource::CullMode::Front:
			return VK_CULL_MODE_FRONT_BIT;
		case GFX::Resource::CullMode::Back:
			return VK_CULL_MODE_BACK_BIT;
		}
	}

	constexpr VkPrimitiveTopology GetPrimitiveTopology(GFX::Resource::TopologyType type, GFX::Resource::TopologyOrder order) noexcept
	{
		switch (type)
		{
		case GFX::Resource::TopologyType::Point:
		{
			switch (order)
			{
			case GFX::Resource::TopologyOrder::List:
				return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
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
				return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case GFX::Resource::TopologyOrder::Strip:
				return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			case GFX::Resource::TopologyOrder::ListAdjacency:
				return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
			case GFX::Resource::TopologyOrder::StripAdjacency:
				return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
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
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case GFX::Resource::TopologyOrder::Strip:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			case GFX::Resource::TopologyOrder::ListAdjacency:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
			case GFX::Resource::TopologyOrder::StripAdjacency:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
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
			case GFX::Resource::TopologyOrder::PatchList2:
			case GFX::Resource::TopologyOrder::PatchList3:
			case GFX::Resource::TopologyOrder::PatchList4:
			case GFX::Resource::TopologyOrder::PatchList5:
			case GFX::Resource::TopologyOrder::PatchList6:
			case GFX::Resource::TopologyOrder::PatchList7:
			case GFX::Resource::TopologyOrder::PatchList8:
			case GFX::Resource::TopologyOrder::PatchList9:
			case GFX::Resource::TopologyOrder::PatchList10:
			case GFX::Resource::TopologyOrder::PatchList11:
			case GFX::Resource::TopologyOrder::PatchList12:
			case GFX::Resource::TopologyOrder::PatchList13:
			case GFX::Resource::TopologyOrder::PatchList14:
			case GFX::Resource::TopologyOrder::PatchList15:
			case GFX::Resource::TopologyOrder::PatchList16:
			case GFX::Resource::TopologyOrder::PatchList17:
			case GFX::Resource::TopologyOrder::PatchList18:
			case GFX::Resource::TopologyOrder::PatchList19:
			case GFX::Resource::TopologyOrder::PatchList20:
			case GFX::Resource::TopologyOrder::PatchList21:
			case GFX::Resource::TopologyOrder::PatchList22:
			case GFX::Resource::TopologyOrder::PatchList23:
			case GFX::Resource::TopologyOrder::PatchList24:
			case GFX::Resource::TopologyOrder::PatchList25:
			case GFX::Resource::TopologyOrder::PatchList26:
			case GFX::Resource::TopologyOrder::PatchList27:
			case GFX::Resource::TopologyOrder::PatchList28:
			case GFX::Resource::TopologyOrder::PatchList29:
			case GFX::Resource::TopologyOrder::PatchList30:
			case GFX::Resource::TopologyOrder::PatchList31:
			case GFX::Resource::TopologyOrder::PatchList32:
				return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
			default:
				ZE_FAIL("Wrong combination of TopologyType and TopologyOrder!");
			}
			break;
		}
		default:
			break;
		}
		return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
	}

	constexpr VkShaderStageFlags GetShaderStage(GFX::Resource::ShaderTypes type) noexcept
	{
		if (type & GFX::Resource::ShaderType::Compute)
			return VK_SHADER_STAGE_COMPUTE_BIT;
		if (type == GFX::Resource::ShaderType::AllGfx)
			return VK_SHADER_STAGE_ALL_GRAPHICS;

		VkShaderStageFlags stage = VK_SHADER_STAGE_ALL;
		if (type & GFX::Resource::ShaderType::Vertex)
			stage |= VK_SHADER_STAGE_VERTEX_BIT;
		if (type & GFX::Resource::ShaderType::Domain)
			stage |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		if (type & GFX::Resource::ShaderType::Hull)
			stage |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		if (type & GFX::Resource::ShaderType::Geometry)
			stage |= VK_SHADER_STAGE_GEOMETRY_BIT;
		if (type & GFX::Resource::ShaderType::Pixel)
			stage |= VK_SHADER_STAGE_FRAGMENT_BIT;

		return stage;
	}

	constexpr VkSamplerAddressMode GetTextureAddressMode(GFX::Resource::Texture::AddressMode mode) noexcept
	{
		switch (mode)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::Texture::AddressMode::Repeat:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case GFX::Resource::Texture::AddressMode::Mirror:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case GFX::Resource::Texture::AddressMode::Edge:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case GFX::Resource::Texture::AddressMode::BorderColor:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case GFX::Resource::Texture::AddressMode::MirrorOnce:
			return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
		}
	}

	constexpr VkPrimitiveTopology GetTopologyType(GFX::Resource::TopologyType type) noexcept
	{
		switch (type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::TopologyType::Undefined:
		case GFX::Resource::TopologyType::ControlPoint:
			return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
		case GFX::Resource::TopologyType::Point:
			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case GFX::Resource::TopologyType::Line:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case GFX::Resource::TopologyType::Triangle:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}
	}

	constexpr U32 GetPatchCount(GFX::Resource::TopologyOrder order) noexcept
	{
		switch (order)
		{
		case GFX::Resource::TopologyOrder::PatchList1:
		case GFX::Resource::TopologyOrder::PatchList2:
		case GFX::Resource::TopologyOrder::PatchList3:
		case GFX::Resource::TopologyOrder::PatchList4:
		case GFX::Resource::TopologyOrder::PatchList5:
		case GFX::Resource::TopologyOrder::PatchList6:
		case GFX::Resource::TopologyOrder::PatchList7:
		case GFX::Resource::TopologyOrder::PatchList8:
		case GFX::Resource::TopologyOrder::PatchList9:
		case GFX::Resource::TopologyOrder::PatchList10:
		case GFX::Resource::TopologyOrder::PatchList11:
		case GFX::Resource::TopologyOrder::PatchList12:
		case GFX::Resource::TopologyOrder::PatchList13:
		case GFX::Resource::TopologyOrder::PatchList14:
		case GFX::Resource::TopologyOrder::PatchList15:
		case GFX::Resource::TopologyOrder::PatchList16:
		case GFX::Resource::TopologyOrder::PatchList17:
		case GFX::Resource::TopologyOrder::PatchList18:
		case GFX::Resource::TopologyOrder::PatchList19:
		case GFX::Resource::TopologyOrder::PatchList20:
		case GFX::Resource::TopologyOrder::PatchList21:
		case GFX::Resource::TopologyOrder::PatchList22:
		case GFX::Resource::TopologyOrder::PatchList23:
		case GFX::Resource::TopologyOrder::PatchList24:
		case GFX::Resource::TopologyOrder::PatchList25:
		case GFX::Resource::TopologyOrder::PatchList26:
		case GFX::Resource::TopologyOrder::PatchList27:
		case GFX::Resource::TopologyOrder::PatchList28:
		case GFX::Resource::TopologyOrder::PatchList29:
		case GFX::Resource::TopologyOrder::PatchList30:
		case GFX::Resource::TopologyOrder::PatchList31:
		case GFX::Resource::TopologyOrder::PatchList32:
			return static_cast<U8>(order) - static_cast<U8>(GFX::Resource::TopologyOrder::PatchList1) + 1;
		default:
			return 0;
		}
	}

	// List of mappings between PixelFormat and VkFormat for enum decoding in X() macro
#define ZE_VK_FORMAT_MAPPINGS \
	X(Unknown,             VK_FORMAT_UNDEFINED) \
	X(R32G32B32A32_Float,  VK_FORMAT_R32G32B32A32_SFLOAT) \
	X(R32G32B32A32_UInt,   VK_FORMAT_R32G32B32A32_UINT) \
	X(R32G32B32A32_SInt,   VK_FORMAT_R32G32B32A32_SINT) \
	X(R16G16B16A16_Float,  VK_FORMAT_R16G16B16A16_SFLOAT) \
	X(R16G16B16A16_UInt,   VK_FORMAT_R16G16B16A16_UINT) \
	X(R16G16B16A16_SInt,   VK_FORMAT_R16G16B16A16_SINT) \
	X(R16G16B16A16_UNorm,  VK_FORMAT_R16G16B16A16_UNORM) \
	X(R16G16B16A16_SNorm,  VK_FORMAT_R16G16B16A16_SNORM) \
	X(R8G8B8A8_UInt,       VK_FORMAT_R8G8B8A8_UINT) \
	X(R8G8B8A8_SInt,       VK_FORMAT_R8G8B8A8_SINT) \
	X(R8G8B8A8_UNorm,      VK_FORMAT_R8G8B8A8_UNORM) \
	X(R8G8B8A8_UNorm_SRGB, VK_FORMAT_R8G8B8A8_SRGB) \
	X(R8G8B8A8_SNorm,      VK_FORMAT_R8G8B8A8_SNORM) \
	X(B8G8R8A8_UNorm,      VK_FORMAT_B8G8R8A8_UNORM) \
	X(B8G8R8A8_UNorm_SRGB, VK_FORMAT_B8G8R8A8_SRGB) \
	X(R32G32B32_Float,     VK_FORMAT_R32G32B32_SFLOAT) \
	X(R32G32B32_UInt,      VK_FORMAT_R32G32B32_UINT) \
	X(R32G32B32_SInt,      VK_FORMAT_R32G32B32_SINT) \
	X(R32G32_Float,        VK_FORMAT_R32G32_SFLOAT) \
	X(R32G32_UInt,         VK_FORMAT_R32G32_UINT) \
	X(R32G32_SInt,         VK_FORMAT_R32G32_SINT) \
	X(R16G16_Float,        VK_FORMAT_R16G16_SFLOAT) \
	X(R16G16_UInt,         VK_FORMAT_R16G16_UINT) \
	X(R16G16_SInt,         VK_FORMAT_R16G16_SINT) \
	X(R16G16_UNorm,        VK_FORMAT_R16G16_UNORM) \
	X(R16G16_SNorm,        VK_FORMAT_R16G16_SNORM) \
	X(R8G8_UInt,           VK_FORMAT_R8G8_UINT) \
	X(R8G8_SInt,           VK_FORMAT_R8G8_SINT) \
	X(R8G8_UNorm,          VK_FORMAT_R8G8_UNORM) \
	X(R8G8_SNorm,          VK_FORMAT_R8G8_SNORM) \
	X(R32_Float,           VK_FORMAT_R32_SFLOAT) \
	X(R32_Depth,           VK_FORMAT_D32_SFLOAT) \
	X(R32_UInt,            VK_FORMAT_R32_UINT) \
	X(R32_SInt,            VK_FORMAT_R32_SINT) \
	X(R16_Float,           VK_FORMAT_R16_SFLOAT) \
	X(R16_UInt,            VK_FORMAT_R16_UINT) \
	X(R16_SInt,            VK_FORMAT_R16_SINT) \
	X(R16_UNorm,           VK_FORMAT_R16_UNORM) \
	X(R16_SNorm,           VK_FORMAT_R16_SNORM) \
	X(R16_Depth,           VK_FORMAT_D16_UNORM) \
	X(R8_UInt,             VK_FORMAT_R8_UINT) \
	X(R8_SInt,             VK_FORMAT_R8_SINT) \
	X(R8_UNorm,            VK_FORMAT_R8_UNORM) \
	X(R8_SNorm,            VK_FORMAT_R8_SNORM) \
	X(R24G8_DepthStencil,  VK_FORMAT_D24_UNORM_S8_UINT) \
	X(R32G8_DepthStencil,  VK_FORMAT_D32_SFLOAT_S8_UINT) \
	X(R10G10B10A2_UInt,    VK_FORMAT_A2B10G10R10_UINT_PACK32) \
	X(R10G10B10A2_UNorm,   VK_FORMAT_A2B10G10R10_UNORM_PACK32) \
	X(R11G11B10_Float,     VK_FORMAT_B10G11R11_UFLOAT_PACK32) \
	X(R9G9B9E5_SharedExp,  VK_FORMAT_E5B9G9R9_UFLOAT_PACK32) \
	X(B4G4R4A4_UNorm,      VK_FORMAT_A4R4G4B4_UNORM_PACK16) \
	X(B5G5R5A1_UNorm,      VK_FORMAT_B5G5R5A1_UNORM_PACK16) \
	X(B5G6R5_UNorm,        VK_FORMAT_B5G6R5_UNORM_PACK16) \
	X(BC1_UNorm,           VK_FORMAT_BC1_RGBA_UNORM_BLOCK) \
	X(BC1_UNorm_SRGB,      VK_FORMAT_BC1_RGBA_SRGB_BLOCK) \
	X(BC2_UNorm,           VK_FORMAT_BC2_UNORM_BLOCK) \
	X(BC2_UNorm_SRGB,      VK_FORMAT_BC2_SRGB_BLOCK) \
	X(BC3_UNorm,           VK_FORMAT_BC3_UNORM_BLOCK) \
	X(BC3_UNorm_SRGB,      VK_FORMAT_BC3_SRGB_BLOCK) \
	X(BC4_UNorm,           VK_FORMAT_BC4_UNORM_BLOCK) \
	X(BC4_SNorm,           VK_FORMAT_BC4_SNORM_BLOCK) \
	X(BC5_UNorm,           VK_FORMAT_BC5_UNORM_BLOCK) \
	X(BC5_SNorm,           VK_FORMAT_BC5_SNORM_BLOCK) \
	X(BC6H_UF16,           VK_FORMAT_BC6H_UFLOAT_BLOCK) \
	X(BC6H_SF16,           VK_FORMAT_BC6H_SFLOAT_BLOCK) \
	X(BC7_UNorm,           VK_FORMAT_BC7_UNORM_BLOCK) \
	X(BC7_UNorm_SRGB,      VK_FORMAT_BC7_SRGB_BLOCK) \
	X(YUV_Y410,            VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16) \
	X(YUV_Y216,            VK_FORMAT_G16B16G16R16_422_UNORM) \
	X(YUV_Y210,            VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16) \
	X(YUV_YUY2,            VK_FORMAT_G8B8G8R8_422_UNORM) \
	X(YUV_P208,            VK_FORMAT_G8_B8R8_2PLANE_422_UNORM) \
	X(YUV_P016,            VK_FORMAT_G16_B16R16_2PLANE_420_UNORM) \
	X(YUV_P010,            VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16) \
	X(YUV_NV12,            VK_FORMAT_G8_B8R8_2PLANE_420_UNORM)

	constexpr VkFormat GetVkFormat(PixelFormat format) noexcept
	{
#define X(pixelFormat, vkFormat) case PixelFormat::##pixelFormat: return vkFormat;
		switch (format)
		{
		default:
			ZE_ENUM_UNHANDLED();
			ZE_VK_FORMAT_MAPPINGS
		}
#undef X
	}

	constexpr PixelFormat GetFormatFromVk(VkFormat format) noexcept
	{
#define X(pixelFormat, vkFormat) case vkFormat: return PixelFormat::##pixelFormat;
		switch (format)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case VK_FORMAT_R4G4_UNORM_PACK8:
		case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
		case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
		case VK_FORMAT_R5G6B5_UNORM_PACK16:
		case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
		case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
		case VK_FORMAT_R8_SRGB:
		case VK_FORMAT_R8G8_SRGB:
		case VK_FORMAT_R8G8B8_UNORM:
		case VK_FORMAT_R8G8B8_SNORM:
		case VK_FORMAT_R8G8B8_UINT:
		case VK_FORMAT_R8G8B8_SINT:
		case VK_FORMAT_R8G8B8_SRGB:
		case VK_FORMAT_B8G8R8_UNORM:
		case VK_FORMAT_B8G8R8_SNORM:
		case VK_FORMAT_B8G8R8_UINT:
		case VK_FORMAT_B8G8R8_SINT:
		case VK_FORMAT_B8G8R8_SRGB:
		case VK_FORMAT_B8G8R8A8_SNORM:
		case VK_FORMAT_B8G8R8A8_UINT:
		case VK_FORMAT_B8G8R8A8_SINT:
		case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
		case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
		case VK_FORMAT_A8B8G8R8_UINT_PACK32:
		case VK_FORMAT_A8B8G8R8_SINT_PACK32:
		case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
		case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
		case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
		case VK_FORMAT_A2R10G10B10_UINT_PACK32:
		case VK_FORMAT_A2R10G10B10_SINT_PACK32:
		case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
		case VK_FORMAT_A2B10G10R10_SINT_PACK32:
		case VK_FORMAT_R16G16B16_UNORM:
		case VK_FORMAT_R16G16B16_SNORM:
		case VK_FORMAT_R16G16B16_UINT:
		case VK_FORMAT_R16G16B16_SINT:
		case VK_FORMAT_R16G16B16_SFLOAT:
		case VK_FORMAT_X8_D24_UNORM_PACK32:
		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
		case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
		case VK_FORMAT_B8G8R8G8_422_UNORM:
		case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
		case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
		case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
		case VK_FORMAT_R10X6_UNORM_PACK16:
		case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
		case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
		case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
		case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
		case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
		case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
		case VK_FORMAT_R12X4_UNORM_PACK16:
		case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
		case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
		case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
		case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
		case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
		case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
		case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
		case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
		case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
		case VK_FORMAT_B16G16R16G16_422_UNORM:
		case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
		case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
		case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
		case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
		case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM:
		case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:
		case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:
		case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM:
		case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
			// 64 bit per channel types are not supported yet by other APIs
		case VK_FORMAT_R64_UINT:
		case VK_FORMAT_R64_SINT:
		case VK_FORMAT_R64_SFLOAT:
		case VK_FORMAT_R64G64_UINT:
		case VK_FORMAT_R64G64_SINT:
		case VK_FORMAT_R64G64_SFLOAT:
		case VK_FORMAT_R64G64B64_UINT:
		case VK_FORMAT_R64G64B64_SINT:
		case VK_FORMAT_R64G64B64_SFLOAT:
		case VK_FORMAT_R64G64B64A64_UINT:
		case VK_FORMAT_R64G64B64A64_SINT:
		case VK_FORMAT_R64G64B64A64_SFLOAT:
			// ETC2 compression not yet supported in other APIs
		case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
		case VK_FORMAT_EAC_R11_UNORM_BLOCK:
		case VK_FORMAT_EAC_R11_SNORM_BLOCK:
		case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
		case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
			// ASTC compression not yet supported in other APIs
		case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
		case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
		case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
		case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
		case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
		case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
		case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
		case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
		case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
		case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
		case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
		case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
		case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
		case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
		case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
		case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
		case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
		case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
		case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
		case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
		case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
		case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
		case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
		case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
		case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
		case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
		case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
		case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
			// VK_EXT_texture_compression_astc_hdr is mostly supported on mobile only with HDR display
		case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK:
			// Scaled formats won't be supported due to API compatibility
		case VK_FORMAT_R8_USCALED:
		case VK_FORMAT_R8_SSCALED:
		case VK_FORMAT_R8G8_USCALED:
		case VK_FORMAT_R8G8_SSCALED:
		case VK_FORMAT_R8G8B8_USCALED:
		case VK_FORMAT_R8G8B8_SSCALED:
		case VK_FORMAT_B8G8R8_USCALED:
		case VK_FORMAT_B8G8R8_SSCALED:
		case VK_FORMAT_R8G8B8A8_USCALED:
		case VK_FORMAT_R8G8B8A8_SSCALED:
		case VK_FORMAT_B8G8R8A8_USCALED:
		case VK_FORMAT_B8G8R8A8_SSCALED:
		case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
		case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
		case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
		case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
		case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
		case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
		case VK_FORMAT_R16_USCALED:
		case VK_FORMAT_R16_SSCALED:
		case VK_FORMAT_R16G16_USCALED:
		case VK_FORMAT_R16G16_SSCALED:
		case VK_FORMAT_R16G16B16_USCALED:
		case VK_FORMAT_R16G16B16_SSCALED:
		case VK_FORMAT_R16G16B16A16_USCALED:
		case VK_FORMAT_R16G16B16A16_SSCALED:
			// VK_NV_optical_flow won't be supported
		case VK_FORMAT_R16G16_S10_5_NV:
			// VK_IMG_format_pvrtc is deprecated
		case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
		case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
		case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
		case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
		case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
		case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
		case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
		case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
			ZE_FAIL("Trying to convert unsupported format!"); [[fallthrough]];
		ZE_VK_FORMAT_MAPPINGS
		case VK_FORMAT_S8_UINT:
			return PixelFormat::R24G8_DepthStencil;
		}
#undef X
	}
#pragma endregion
}