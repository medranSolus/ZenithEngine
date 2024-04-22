#pragma once
// Headers needed for DirectX 12
#include "RHI/DX/DXGI.h"
#include "GFX/Binding/Range.h"
#include "GFX/Pipeline/Barrier.h"
#include "GFX/Pipeline/TextureLayout.h"
#include "GFX/Resource/PipelineStateDesc.h"
#include "GFX/Resource/SamplerDesc.h"
#include "GFX/ShaderPresence.h"
#include "GFX/QueueType.h"
ZE_WARNING_PUSH
#include "d3d12.h"
ZE_WARNING_POP
#include <bitset>

namespace ZE::RHI::DX12
{
	// Wrappers for DirectX 12 interfaces (to avoid using multiple different versions)
	typedef ID3D12CommandAllocator                   ICommandAllocator;
	typedef ID3D12CommandList                        ICommandList;
	typedef ID3D12CommandSignature                   ICommandSignature;
	typedef ID3D12CommandQueue                       ICommandQueue;
	typedef ID3D12DescriptorHeap                     IDescriptorHeap;
	typedef ID3D12Debug6                             IDebug;
	typedef ID3D12DebugDevice2                       IDebugDevice;
	typedef ID3D12Device14                           IDevice;
	typedef ID3D12DeviceRemovedExtendedData1         IDeviceRemovedExtendedData;
	typedef ID3D12DeviceRemovedExtendedDataSettings1 IDeviceRemovedExtendedDataSettings;
	typedef ID3D12Fence1                             IFence;
	typedef ID3D12GraphicsCommandList10              IGraphicsCommandList;
	typedef ID3D12Heap1                              IHeap;
	typedef ID3D12InfoQueue                          IInfoQueue;
	typedef ID3D12Pageable                           IPageable;
	typedef ID3D12PipelineState                      IPipelineState;
	typedef ID3D12QueryHeap                          IQueryHeap;
	typedef ID3D12Resource2                          IResource;
	typedef ID3D12RootSignature                      IRootSignature;

	// Get DirectX 12 version of resource access
	constexpr D3D12_BARRIER_ACCESS GetBarrierAccess(GFX::Pipeline::ResourceAccesses accesses) noexcept;
	// Get DirectX 12 version of pipeline texture layout
	constexpr D3D12_BARRIER_LAYOUT GetBarrierLayout(GFX::Pipeline::TextureLayout layout) noexcept;
	// Get DirectX 12 version of pipeline barrier sync
	constexpr D3D12_BARRIER_SYNC GetBarrierSync(GFX::Pipeline::StageSyncs syncs) noexcept;
	// Get DirectX 12 version of command list types
	constexpr D3D12_COMMAND_LIST_TYPE GetCommandType(GFX::QueueType type) noexcept;
	// Get DirectX 12 version of comparison function
	constexpr D3D12_COMPARISON_FUNC GetComparisonFunc(GFX::Resource::CompareMethod func) noexcept;
	// Get DirectX 12 version of culling modes
	constexpr D3D12_CULL_MODE GetCulling(GFX::Resource::CullMode mode) noexcept;
	// Get DirectX 12 version of filter type
	constexpr D3D12_FILTER GetFilterType(GFX::Resource::SamplerFilter samplerType) noexcept;
	// Get register space for given shader type
	constexpr U32 GetRegisterSpaceForShader(GFX::Binding::RangeFlags flags, GFX::Resource::ShaderTypes type) noexcept;
	// Get shader visibility from shader type
	constexpr D3D12_SHADER_VISIBILITY GetShaderVisibility(GFX::Resource::ShaderTypes type, GFX::ShaderPresenceMask* presence = nullptr) noexcept;
	// Get DirectX 12 version of static border color for static sampler
	constexpr D3D12_STATIC_BORDER_COLOR GetStaticBorderColor(GFX::Resource::Texture::EdgeColor color) noexcept;
	// Get DirectX 12 version of texture addressing mode
	constexpr D3D12_TEXTURE_ADDRESS_MODE GetTextureAddressMode(GFX::Resource::Texture::AddressMode mode) noexcept;
	// Get DirectX 12 version of primitive topology types
	constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE GetTopologyType(GFX::Resource::TopologyType type) noexcept;

#pragma region Functions
	constexpr D3D12_BARRIER_ACCESS GetBarrierAccess(GFX::Pipeline::ResourceAccesses accesses) noexcept
	{
		if (accesses == GFX::Pipeline::ResourceAccess::None)
			return D3D12_BARRIER_ACCESS_NO_ACCESS;

		D3D12_BARRIER_ACCESS barrierAccess = D3D12_BARRIER_ACCESS_COMMON;
		if (accesses & GFX::Pipeline::ResourceAccess::VertexBuffer)
			barrierAccess |= D3D12_BARRIER_ACCESS_VERTEX_BUFFER;
		if (accesses & GFX::Pipeline::ResourceAccess::ConstantBuffer)
			barrierAccess |= D3D12_BARRIER_ACCESS_CONSTANT_BUFFER;
		if (accesses & GFX::Pipeline::ResourceAccess::IndexBuffer)
			barrierAccess |= D3D12_BARRIER_ACCESS_INDEX_BUFFER;
		if (accesses & GFX::Pipeline::ResourceAccess::RenderTarget)
			barrierAccess |= D3D12_BARRIER_ACCESS_RENDER_TARGET;
		if (accesses & GFX::Pipeline::ResourceAccess::UnorderedAccess)
			barrierAccess |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
		if (accesses & GFX::Pipeline::ResourceAccess::DepthStencilWrite)
			barrierAccess |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
		if (accesses & GFX::Pipeline::ResourceAccess::DepthStencilRead)
			barrierAccess |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
		if (accesses & GFX::Pipeline::ResourceAccess::ShaderResource)
			barrierAccess |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
		if (accesses & GFX::Pipeline::ResourceAccess::StreamOutput)
			barrierAccess |= D3D12_BARRIER_ACCESS_STREAM_OUTPUT;
		if (accesses & GFX::Pipeline::ResourceAccess::IndirectArguments)
			barrierAccess |= D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT;
		if (accesses & GFX::Pipeline::ResourceAccess::Predication)
			barrierAccess |= D3D12_BARRIER_ACCESS_PREDICATION;
		if (accesses & GFX::Pipeline::ResourceAccess::CopyDest)
			barrierAccess |= D3D12_BARRIER_ACCESS_COPY_DEST;
		if (accesses & GFX::Pipeline::ResourceAccess::CopySource)
			barrierAccess |= D3D12_BARRIER_ACCESS_COPY_SOURCE;
		if (accesses & GFX::Pipeline::ResourceAccess::ResolveDest)
			barrierAccess |= D3D12_BARRIER_ACCESS_RESOLVE_DEST;
		if (accesses & GFX::Pipeline::ResourceAccess::ResolveSource)
			barrierAccess |= D3D12_BARRIER_ACCESS_RESOLVE_SOURCE;
		if (accesses & GFX::Pipeline::ResourceAccess::RayTracingAccelerationStructRead)
			barrierAccess |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ;
		if (accesses & GFX::Pipeline::ResourceAccess::RayTracingAccelerationStructWrite)
			barrierAccess |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE;
		if (accesses & GFX::Pipeline::ResourceAccess::ShadingRateSource)
			barrierAccess |= D3D12_BARRIER_ACCESS_SHADING_RATE_SOURCE;

		/* Not used:
		 * D3D12_BARRIER_ACCESS_VIDEO_DECODE_READ
		 * D3D12_BARRIER_ACCESS_VIDEO_DECODE_WRITE
		 * D3D12_BARRIER_ACCESS_VIDEO_PROCESS_READ
		 * D3D12_BARRIER_ACCESS_VIDEO_PROCESS_WRITE
		 * D3D12_BARRIER_ACCESS_VIDEO_ENCODE_READ
		 * D3D12_BARRIER_ACCESS_VIDEO_ENCODE_WRITE
		 */
		return barrierAccess;
	}

	constexpr D3D12_BARRIER_LAYOUT GetBarrierLayout(GFX::Pipeline::TextureLayout layout) noexcept
	{
		switch (layout)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Pipeline::TextureLayout::Undefined:
		case GFX::Pipeline::TextureLayout::Preinitialized:
			return D3D12_BARRIER_LAYOUT_UNDEFINED;
		case GFX::Pipeline::TextureLayout::Common:
			return D3D12_BARRIER_LAYOUT_COMMON;
		case GFX::Pipeline::TextureLayout::Present:
			return D3D12_BARRIER_LAYOUT_PRESENT;
		case GFX::Pipeline::TextureLayout::GenericRead:
			return D3D12_BARRIER_LAYOUT_GENERIC_READ;
		case GFX::Pipeline::TextureLayout::RenderTarget:
			return D3D12_BARRIER_LAYOUT_RENDER_TARGET;
		case GFX::Pipeline::TextureLayout::UnorderedAccess:
			return D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
		case GFX::Pipeline::TextureLayout::DepthStencilWrite:
			return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
		case GFX::Pipeline::TextureLayout::DepthStencilRead:
			return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
		case GFX::Pipeline::TextureLayout::ShaderResource:
			return D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
		case GFX::Pipeline::TextureLayout::CopySource:
			return D3D12_BARRIER_LAYOUT_COPY_SOURCE;
		case GFX::Pipeline::TextureLayout::CopyDest:
			return D3D12_BARRIER_LAYOUT_COPY_DEST;
		case GFX::Pipeline::TextureLayout::ResolveSource:
			return D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE;
		case GFX::Pipeline::TextureLayout::ResolveDest:
			return D3D12_BARRIER_LAYOUT_RESOLVE_DEST;
		case GFX::Pipeline::TextureLayout::ShadingRateSource:
			return D3D12_BARRIER_LAYOUT_SHADING_RATE_SOURCE;
		}
	}

	constexpr D3D12_BARRIER_SYNC GetBarrierSync(GFX::Pipeline::StageSyncs syncs) noexcept
	{
		D3D12_BARRIER_SYNC barrierSync = D3D12_BARRIER_SYNC_NONE;
		if (syncs & GFX::Pipeline::StageSync::All)
			barrierSync |= D3D12_BARRIER_SYNC_ALL;
		if (syncs & GFX::Pipeline::StageSync::AllGraphics)
			barrierSync |= D3D12_BARRIER_SYNC_DRAW;
		if (syncs & GFX::Pipeline::StageSync::IndexInput)
			barrierSync |= D3D12_BARRIER_SYNC_INDEX_INPUT;
		if (syncs & GFX::Pipeline::StageSync::GeometryShading)
			barrierSync |= D3D12_BARRIER_SYNC_VERTEX_SHADING;
		if (syncs & GFX::Pipeline::StageSync::PixelShading)
			barrierSync |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
		if (syncs & GFX::Pipeline::StageSync::ComputeShading)
			barrierSync |= D3D12_BARRIER_SYNC_COMPUTE_SHADING;
		if (syncs & GFX::Pipeline::StageSync::RayTracing)
			barrierSync |= D3D12_BARRIER_SYNC_RAYTRACING;
		if (syncs & GFX::Pipeline::StageSync::ExecuteIndirect)
			barrierSync |= D3D12_BARRIER_SYNC_EXECUTE_INDIRECT;
		if (syncs & GFX::Pipeline::StageSync::RenderTarget)
			barrierSync |= D3D12_BARRIER_SYNC_RENDER_TARGET;
		if (syncs & GFX::Pipeline::StageSync::DepthStencil)
			barrierSync |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;
		if (syncs & GFX::Pipeline::StageSync::ClearUnorderedAccess)
			barrierSync |= D3D12_BARRIER_SYNC_CLEAR_UNORDERED_ACCESS_VIEW;
		if (syncs & GFX::Pipeline::StageSync::Copy)
			barrierSync |= D3D12_BARRIER_SYNC_COPY;
		if (syncs & GFX::Pipeline::StageSync::Resolve)
			barrierSync |= D3D12_BARRIER_SYNC_RESOLVE;
		if (syncs & GFX::Pipeline::StageSync::Predication)
			barrierSync |= D3D12_BARRIER_SYNC_PREDICATION;
		if (syncs & GFX::Pipeline::StageSync::RayTracingAccelerationStructBuild)
			barrierSync |= D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE;
		if (syncs & GFX::Pipeline::StageSync::RayTracingAccelerationStructCopy)
			barrierSync |= D3D12_BARRIER_SYNC_COPY_RAYTRACING_ACCELERATION_STRUCTURE;

		/* Not used:
		 * D3D12_BARRIER_SYNC_ALL_SHADING
		 * D3D12_BARRIER_SYNC_NON_PIXEL_SHADING
		 * D3D12_BARRIER_SYNC_EMIT_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO
		 * D3D12_BARRIER_SYNC_VIDEO_DECODE
		 * D3D12_BARRIER_SYNC_VIDEO_PROCESS
		 * D3D12_BARRIER_SYNC_VIDEO_ENCODE
		 */
		return barrierSync;
	}

	constexpr D3D12_COMMAND_LIST_TYPE GetCommandType(GFX::QueueType type) noexcept
	{
		switch (type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::QueueType::Main:
			return D3D12_COMMAND_LIST_TYPE_DIRECT;
		case GFX::QueueType::Compute:
			return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		case GFX::QueueType::Copy:
			return D3D12_COMMAND_LIST_TYPE_COPY;
		}
	}

	constexpr D3D12_COMPARISON_FUNC GetComparisonFunc(GFX::Resource::CompareMethod func) noexcept
	{
		switch (func)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::CompareMethod::Never:
			return D3D12_COMPARISON_FUNC_NEVER;
		case GFX::Resource::CompareMethod::Less:
			return D3D12_COMPARISON_FUNC_LESS;
		case GFX::Resource::CompareMethod::Equal:
			return D3D12_COMPARISON_FUNC_EQUAL;
		case GFX::Resource::CompareMethod::LessEqual:
			return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case GFX::Resource::CompareMethod::Greater:
			return D3D12_COMPARISON_FUNC_GREATER;
		case GFX::Resource::CompareMethod::NotEqual:
			return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case GFX::Resource::CompareMethod::GreaterEqual:
			return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case GFX::Resource::CompareMethod::Always:
			return D3D12_COMPARISON_FUNC_ALWAYS;
		}
	}

	constexpr D3D12_CULL_MODE GetCulling(GFX::Resource::CullMode mode) noexcept
	{
		switch (mode)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::CullMode::None:
			return D3D12_CULL_MODE_NONE;
		case GFX::Resource::CullMode::Front:
			return D3D12_CULL_MODE_FRONT;
		case GFX::Resource::CullMode::Back:
			return D3D12_CULL_MODE_BACK;
		}
	}

	constexpr D3D12_FILTER GetFilterType(GFX::Resource::SamplerFilter samplerType) noexcept
	{
		if (samplerType == GFX::Resource::SamplerType::Default)
			return D3D12_FILTER_MIN_MAG_MIP_POINT; // 000
		if (samplerType == GFX::Resource::SamplerType::Linear)
			return D3D12_FILTER_MIN_MAG_MIP_LINEAR; // 111
		if (samplerType == GFX::Resource::SamplerType::Anisotropic)
			return D3D12_FILTER_ANISOTROPIC;

		switch (samplerType & GFX::Resource::SamplerType::OperationType)
		{
		case GFX::Resource::SamplerType::Comparison:
		{
			if ((samplerType & ~GFX::Resource::SamplerType::OperationType) == GFX::Resource::SamplerType::Point)
				return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT; // 000
			if ((samplerType & ~GFX::Resource::SamplerType::OperationType) == GFX::Resource::SamplerType::Linear)
				return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // 111
			if (samplerType & GFX::Resource::SamplerType::Anisotropic)
				return D3D12_FILTER_COMPARISON_ANISOTROPIC;

			if (samplerType & GFX::Resource::SamplerType::LinearMipSampling)
			{
				if (samplerType & GFX::Resource::SamplerType::LinearMagnification)
					return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR; // 110
				if (samplerType & GFX::Resource::SamplerType::LinearMinification)
					return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR; // 101
				return D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR; // 100
			}
			if (samplerType & GFX::Resource::SamplerType::LinearMagnification)
			{
				if (samplerType & GFX::Resource::SamplerType::LinearMinification)
					return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT; // 011
				return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT; // 010
			}
			return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT; // 001
		}
		case GFX::Resource::SamplerType::Minimum:
		{
			if ((samplerType & ~GFX::Resource::SamplerType::OperationType) == GFX::Resource::SamplerType::Point)
				return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_POINT; // 000
			if ((samplerType & ~GFX::Resource::SamplerType::OperationType) == GFX::Resource::SamplerType::Linear)
				return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR; // 111
			if (samplerType & GFX::Resource::SamplerType::Anisotropic)
				return D3D12_FILTER_MINIMUM_ANISOTROPIC;

			if (samplerType & GFX::Resource::SamplerType::LinearMipSampling)
			{
				if (samplerType & GFX::Resource::SamplerType::LinearMagnification)
					return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR; // 110
				if (samplerType & GFX::Resource::SamplerType::LinearMinification)
					return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR; // 101
				return D3D12_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR; // 100
			}
			if (samplerType & GFX::Resource::SamplerType::LinearMagnification)
			{
				if (samplerType & GFX::Resource::SamplerType::LinearMinification)
					return D3D12_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT; // 011
				return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT; // 010
			}
			return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT; // 001
		}
		case GFX::Resource::SamplerType::Maximum:
		{
			if ((samplerType & ~GFX::Resource::SamplerType::OperationType) == GFX::Resource::SamplerType::Point)
				return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_POINT; // 000
			if ((samplerType & ~GFX::Resource::SamplerType::OperationType) == GFX::Resource::SamplerType::Linear)
				return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR; // 111
			if (samplerType & GFX::Resource::SamplerType::Anisotropic)
				return D3D12_FILTER_MAXIMUM_ANISOTROPIC;

			if (samplerType & GFX::Resource::SamplerType::LinearMipSampling)
			{
				if (samplerType & GFX::Resource::SamplerType::LinearMagnification)
					return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR; // 110
				if (samplerType & GFX::Resource::SamplerType::LinearMinification)
					return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR; // 101
				return D3D12_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR; // 100
			}
			if (samplerType & GFX::Resource::SamplerType::LinearMagnification)
			{
				if (samplerType & GFX::Resource::SamplerType::LinearMinification)
					return D3D12_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT; // 011
				return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT; // 010
			}
			return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT; // 001
		}
		}

		if (samplerType & GFX::Resource::SamplerType::LinearMipSampling)
		{
			if (samplerType & GFX::Resource::SamplerType::LinearMagnification)
				return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR; // 110
			if (samplerType & GFX::Resource::SamplerType::LinearMinification)
				return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR; // 101
			return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR; // 100
		}
		if (samplerType & GFX::Resource::SamplerType::LinearMagnification)
		{
			if (samplerType & GFX::Resource::SamplerType::LinearMinification)
				return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT; // 011
			return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT; // 010
		}
		return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT; // 001
	}

	constexpr U32 GetRegisterSpaceForShader(GFX::Binding::RangeFlags flags, GFX::Resource::ShaderTypes type) noexcept
	{
		ZE_ASSERT(type, "Empty shader type!");

		if (flags & GFX::Binding::RangeFlag::GlobalBuffer)
			return 0;

		switch (type)
		{
		case GFX::Resource::ShaderType::Vertex:
			return 1;
		case GFX::Resource::ShaderType::Domain:
			return 3;
		case GFX::Resource::ShaderType::Hull:
			return 4;
		case GFX::Resource::ShaderType::Geometry:
			return 2;
		default:
			return 0;
		}
	}

	constexpr D3D12_SHADER_VISIBILITY GetShaderVisibility(GFX::Resource::ShaderTypes type, GFX::ShaderPresenceMask* presence) noexcept
	{
		if (type & GFX::Resource::ShaderType::Compute)
		{
			if (presence)
				presence->SetCompute();
			return D3D12_SHADER_VISIBILITY_ALL;
		}
		if (type == GFX::Resource::ShaderType::AllGfx)
		{
			if (presence)
				presence->SetGfx();
			return D3D12_SHADER_VISIBILITY_ALL;
		}

		D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL;
		if (type & GFX::Resource::ShaderType::Vertex)
		{
			if (presence)
				presence->SetVertex();
			visibility = D3D12_SHADER_VISIBILITY_VERTEX;
		}
		if (type & GFX::Resource::ShaderType::Domain)
		{
			if (presence)
				presence->SetDomain();
			visibility = D3D12_SHADER_VISIBILITY_DOMAIN;
		}
		if (type & GFX::Resource::ShaderType::Hull)
		{
			if (presence)
				presence->SetHull();
			visibility = D3D12_SHADER_VISIBILITY_HULL;
		}
		if (type & GFX::Resource::ShaderType::Geometry)
		{
			if (presence)
				presence->SetGeometry();
			visibility = D3D12_SHADER_VISIBILITY_GEOMETRY;
		}
		if (type & GFX::Resource::ShaderType::Pixel)
		{
			if (presence)
				presence->SetPixel();
			visibility = D3D12_SHADER_VISIBILITY_PIXEL;
		}

		return Math::IsPower2(type) ? visibility : D3D12_SHADER_VISIBILITY_ALL;
	}

	constexpr D3D12_STATIC_BORDER_COLOR GetStaticBorderColor(GFX::Resource::Texture::EdgeColor color) noexcept
	{
		switch (color)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::Texture::EdgeColor::TransparentBlack:
			return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		case GFX::Resource::Texture::EdgeColor::SolidBlack:
			return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		case GFX::Resource::Texture::EdgeColor::SolidWhite:
			return D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		}
	}

	constexpr D3D12_TEXTURE_ADDRESS_MODE GetTextureAddressMode(GFX::Resource::Texture::AddressMode mode) noexcept
	{
		switch (mode)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::Texture::AddressMode::Repeat:
			return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		case GFX::Resource::Texture::AddressMode::Mirror:
			return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		case GFX::Resource::Texture::AddressMode::Edge:
			return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		case GFX::Resource::Texture::AddressMode::BorderColor:
			return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		case GFX::Resource::Texture::AddressMode::MirrorOnce:
			return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
		}
	}

	constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE GetTopologyType(GFX::Resource::TopologyType type) noexcept
	{
		switch (type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::TopologyType::Undefined:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
		case GFX::Resource::TopologyType::Point:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		case GFX::Resource::TopologyType::Line:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case GFX::Resource::TopologyType::Triangle:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		case GFX::Resource::TopologyType::ControlPoint:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
		}
	}
#pragma endregion
}