#pragma once
// Headers needed for DirectX 12
#include "GFX/API/DX/DXGI.h"
#include "GFX/Binding/Range.h"
#include "GFX/Pipeline/BarrierType.h"
#include "GFX/Resource/PipelineStateDesc.h"
#include "GFX/Resource/SamplerDesc.h"
#include "GFX/Resource/ShaderType.h"
#include "GFX/Resource/State.h"
#include "GFX/QueueType.h"
#include "WarningGuardOn.h"
#include <d3d12.h>
#include "WarningGuardOff.h"
#include <bitset>

namespace ZE::GFX::API::DX12
{
	// Wrappers for DirectX 12 interfaces (to avoid using multiple different versions)
	typedef ID3D12CommandAllocator                   ICommandAllocator;
	typedef ID3D12CommandList                        ICommandList;
	typedef ID3D12CommandQueue                       ICommandQueue;
	typedef ID3D12DescriptorHeap                     IDescriptorHeap;
	typedef ID3D12Debug1                             IDebug;
	typedef ID3D12DebugDevice2                       IDebugDevice;
	typedef ID3D12Device8                            IDevice;
	typedef ID3D12DeviceRemovedExtendedData1         IDeviceRemovedExtendedData;
	typedef ID3D12DeviceRemovedExtendedDataSettings1 IDeviceRemovedExtendedDataSettings;
	typedef ID3D12Fence1                             IFence;
	typedef ID3D12GraphicsCommandList6               IGraphicsCommandList;
	typedef ID3D12Heap1                              IHeap;
	typedef ID3D12InfoQueue                          IInfoQueue;
	typedef ID3D12Pageable                           IPageable;
	typedef ID3D12PipelineState                      IPipelineState;
	typedef ID3D12QueryHeap                          IQueryHeap;
	typedef ID3D12Resource2                          IResource;
	typedef ID3D12RootSignature                      IRootSignature;

	typedef std::bitset<6> ShaderPresenceMask;

	// Get DirectX 12 version of command list types
	constexpr D3D12_COMMAND_LIST_TYPE GetCommandType(QueueType type) noexcept;
	// Get DirectX 12 version of comparison function
	constexpr D3D12_COMPARISON_FUNC GetComparisonFunc(GFX::Resource::CompareMethod func) noexcept;
	// Get DirectX 12 version of culling modes
	constexpr D3D12_CULL_MODE GetCulling(GFX::Resource::CullMode mode) noexcept;
	// Get DirectX 12 version of filter type
	constexpr D3D12_FILTER GetFilterType(U8 samplerType) noexcept;
	// Get register space for given shader type
	constexpr U32 GetRegisterSpaceForShader(GFX::Binding::RangeFlags flags, GFX::Resource::ShaderTypes type) noexcept;
	// Get DirectX 12 version of resource states
	constexpr D3D12_RESOURCE_STATES GetResourceState(GFX::Resource::State state) noexcept;
	// Get shader visibility from shader type
	constexpr D3D12_SHADER_VISIBILITY GetShaderVisibility(GFX::Resource::ShaderTypes type, ShaderPresenceMask* presence = nullptr) noexcept;
	// Get DirectX 12 version of static border color for static sampler
	constexpr D3D12_STATIC_BORDER_COLOR GetStaticBorderColor(GFX::Resource::Texture::EdgeColor color) noexcept;
	// Get DirectX 12 version of texture addressing mode
	constexpr D3D12_TEXTURE_ADDRESS_MODE GetTextureAddressMode(GFX::Resource::Texture::AddressMode mode) noexcept;
	// Get DirectX 12 version of primitive topology types
	constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE GetTopologyType(GFX::Resource::TopologyType type) noexcept;
	// Get DirectX 12 version of possible barrier types
	constexpr D3D12_RESOURCE_BARRIER_FLAGS GetTransitionType(GFX::Pipeline::BarrierType type) noexcept;

#pragma region Functions
	constexpr D3D12_COMMAND_LIST_TYPE GetCommandType(QueueType type) noexcept
	{
		switch (type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case QueueType::Main:
			return D3D12_COMMAND_LIST_TYPE_DIRECT;
		case QueueType::Compute:
			return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		case QueueType::Copy:
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

	constexpr D3D12_FILTER GetFilterType(U8 samplerType) noexcept
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

	constexpr D3D12_RESOURCE_STATES GetResourceState(GFX::Resource::State state) noexcept
	{
		D3D12_RESOURCE_STATES resState = D3D12_RESOURCE_STATE_COMMON;

		if (state == GFX::Resource::StateGenericRead)
			resState = D3D12_RESOURCE_STATE_GENERIC_READ;
		else
		{
			if (state & GFX::Resource::StateVertexBuffer || state & GFX::Resource::StateConstantBuffer)
				resState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
			if (state & GFX::Resource::StateIndexBuffer)
				resState |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
			if (state & GFX::Resource::StateShaderResourceNonPS)
				resState |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			if (state & GFX::Resource::StateShaderResourcePS)
				resState |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			if (state & GFX::Resource::StateCopySource)
				resState |= D3D12_RESOURCE_STATE_COPY_SOURCE;
			if (state & GFX::Resource::StateIndirect)
				resState |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
		}
		if (state & GFX::Resource::StateDepthRead)
			resState |= D3D12_RESOURCE_STATE_DEPTH_READ;

		if (state & GFX::Resource::StateRenderTarget)
			resState |= D3D12_RESOURCE_STATE_RENDER_TARGET;
		if (state & GFX::Resource::StateCopyDestination)
			resState |= D3D12_RESOURCE_STATE_COPY_DEST;
		if (state & GFX::Resource::StateStreamOut)
			resState |= D3D12_RESOURCE_STATE_STREAM_OUT;
		if (state & GFX::Resource::StateDepthWrite)
			resState |= D3D12_RESOURCE_STATE_DEPTH_WRITE;

		if (state & GFX::Resource::StateUnorderedAccess)
			resState |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		if (state & GFX::Resource::StateAccelerationStructureRT)
			resState |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
		if (state & GFX::Resource::StateShadingRateSource)
			resState |= D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
		if (state & GFX::Resource::StatePredication)
			resState |= D3D12_RESOURCE_STATE_PREDICATION;

		if (state & GFX::Resource::StateResolveDestination)
			resState |= D3D12_RESOURCE_STATE_RESOLVE_DEST;
		if (state & GFX::Resource::StateResolveSource)
			resState |= D3D12_RESOURCE_STATE_RESOLVE_SOURCE;

		if (state & GFX::Resource::StateVideoDecodeRead)
			resState |= D3D12_RESOURCE_STATE_VIDEO_DECODE_READ;
		if (state & GFX::Resource::StateVideoProcessRead)
			resState |= D3D12_RESOURCE_STATE_VIDEO_PROCESS_READ;
		if (state & GFX::Resource::StateVideoEncodeRead)
			resState |= D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ;

		if (state & GFX::Resource::StateVideoDecodeWrite)
			resState |= D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE;
		if (state & GFX::Resource::StateVideoProcessWrite)
			resState |= D3D12_RESOURCE_STATE_VIDEO_PROCESS_WRITE;
		if (state & GFX::Resource::StateVideoEncodeWrite)
			resState |= D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE;

		return resState;
	}

	constexpr D3D12_SHADER_VISIBILITY GetShaderVisibility(GFX::Resource::ShaderTypes type, ShaderPresenceMask* presence) noexcept
	{
		if (type & GFX::Resource::ShaderType::Compute)
		{
			if (presence)
				(*presence)[5] = true;
			return D3D12_SHADER_VISIBILITY_ALL;
		}
		if (type == GFX::Resource::ShaderType::AllGfx)
		{
			if (presence)
				*presence = GFX::Resource::ShaderType::AllGfx;
			return D3D12_SHADER_VISIBILITY_ALL;
		}

		D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL;
		if (type & GFX::Resource::ShaderType::Vertex)
		{
			if (presence)
				(*presence)[0] = true;
			visibility = D3D12_SHADER_VISIBILITY_VERTEX;
		}
		if (type & GFX::Resource::ShaderType::Domain)
		{
			if (presence)
				(*presence)[1] = true;
			visibility = D3D12_SHADER_VISIBILITY_DOMAIN;
		}
		if (type & GFX::Resource::ShaderType::Hull)
		{
			if (presence)
				(*presence)[2] = true;
			visibility = D3D12_SHADER_VISIBILITY_HULL;
		}
		if (type & GFX::Resource::ShaderType::Geometry)
		{
			if (presence)
				(*presence)[3] = true;
			visibility = D3D12_SHADER_VISIBILITY_GEOMETRY;
		}
		if (type & GFX::Resource::ShaderType::Pixel)
		{
			if (presence)
				(*presence)[4] = true;
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

	constexpr D3D12_RESOURCE_BARRIER_FLAGS GetTransitionType(GFX::Pipeline::BarrierType type) noexcept
	{
		switch (type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Pipeline::BarrierType::Immediate:
			return D3D12_RESOURCE_BARRIER_FLAG_NONE;
		case GFX::Pipeline::BarrierType::Begin:
			return D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
		case GFX::Pipeline::BarrierType::End:
			return D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
		}
	}
#pragma endregion
}