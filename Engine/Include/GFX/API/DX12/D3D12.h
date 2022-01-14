#pragma once
// Headers needed for DirectX 12
#include "GFX/API/DX/DXGI.h"
#include "GFX/Pipeline/BarrierType.h"
#include "GFX/Resource/PipelineStateDesc.h"
#include "GFX/Resource/SamplerDesc.h"
#include "GFX/Resource/ShaderType.h"
#include "GFX/Resource/State.h"
#include "GFX/CommandType.h"
#include <d3d12.h>
#include <bitset>

namespace ZE::GFX::API::DX12
{
	// Get DirectX 12 version of command list types
	constexpr D3D12_COMMAND_LIST_TYPE GetCommandType(CommandType type) noexcept;
	// Get DirectX 12 version of comparison function
	constexpr D3D12_COMPARISON_FUNC GetComparisonFunc(GFX::Resource::CompareMethod func) noexcept;
	// Get DirectX 12 version of culling modes
	constexpr D3D12_CULL_MODE GetCulling(GFX::Resource::CullMode mode) noexcept;
	// Get DirectX 12 version of filter type
	constexpr D3D12_FILTER GetFilterType(U8 samplerType) noexcept;
	// Get register space for given shader type
	constexpr U32 GetRegisterSpaceForShader(GFX::Resource::ShaderType type) noexcept;
	// Get DirectX 12 version of resource states
	constexpr D3D12_RESOURCE_STATES GetResourceState(GFX::Resource::State state) noexcept;
	// Get shader visibility from shader type
	constexpr D3D12_SHADER_VISIBILITY GetShaderVisibility(GFX::Resource::ShaderType type, std::bitset<6>* presence = nullptr) noexcept;
	// Get DirectX 12 version of static border color for static sampler
	constexpr D3D12_STATIC_BORDER_COLOR GetStaticBorderColor(GFX::Resource::Texture::EdgeColor color) noexcept;
	// Get DirectX 12 version of texture addressing mode
	constexpr D3D12_TEXTURE_ADDRESS_MODE GetTextureAddressMode(GFX::Resource::Texture::AddressMode mode) noexcept;
	// Get DirectX 12 version of primitive topology types
	constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE GetTopologyType(GFX::Resource::TopologyType type) noexcept;
	// Get DirectX 12 version of possible barrier types
	constexpr D3D12_RESOURCE_BARRIER_FLAGS GetTransitionType(GFX::Pipeline::BarrierType type) noexcept;

#pragma region Functions
	constexpr D3D12_COMMAND_LIST_TYPE GetCommandType(CommandType type) noexcept
	{
		switch (type)
		{
		case CommandType::Bundle:
			return D3D12_COMMAND_LIST_TYPE_BUNDLE;
		case CommandType::Compute:
			return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		case CommandType::Copy:
			return D3D12_COMMAND_LIST_TYPE_COPY;
		}
		return D3D12_COMMAND_LIST_TYPE_DIRECT;
	}

	constexpr D3D12_COMPARISON_FUNC GetComparisonFunc(GFX::Resource::CompareMethod func) noexcept
	{
		switch (func)
		{
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
		return D3D12_COMPARISON_FUNC_NEVER;
	}

	constexpr D3D12_CULL_MODE GetCulling(GFX::Resource::CullMode mode) noexcept
	{
		switch (mode)
		{
		case GFX::Resource::CullMode::Front:
			return D3D12_CULL_MODE_FRONT;
		case GFX::Resource::CullMode::Back:
			return D3D12_CULL_MODE_BACK;
		}
		return D3D12_CULL_MODE_NONE;
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

	constexpr U32 GetRegisterSpaceForShader(GFX::Resource::ShaderType type) noexcept
	{
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
		}
		return 0;
	}

	constexpr D3D12_RESOURCE_STATES GetResourceState(GFX::Resource::State state) noexcept
	{
		switch (state)
		{
		case GFX::Resource::State::GenericRead:
			return D3D12_RESOURCE_STATE_GENERIC_READ;
		case GFX::Resource::State::Present:
			return D3D12_RESOURCE_STATE_PRESENT;
		case GFX::Resource::State::VertexBuffer:
		case GFX::Resource::State::ConstantBuffer:
			return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		case GFX::Resource::State::IndexBuffer:
			return D3D12_RESOURCE_STATE_INDEX_BUFFER;
		case GFX::Resource::State::ShaderResourceNonPS:
			return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		case GFX::Resource::State::ShaderResourcePS:
			return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		case GFX::Resource::State::ShaderResourceAll:
			return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		case GFX::Resource::State::DepthRead:
			return D3D12_RESOURCE_STATE_DEPTH_READ;
		case GFX::Resource::State::RenderTarget:
			return D3D12_RESOURCE_STATE_RENDER_TARGET;
		case GFX::Resource::State::UnorderedAccess:
			return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		case GFX::Resource::State::DepthWrite:
			return D3D12_RESOURCE_STATE_DEPTH_WRITE;
		case GFX::Resource::State::CopyDestination:
			return D3D12_RESOURCE_STATE_COPY_DEST;
		case GFX::Resource::State::CopySource:
			return D3D12_RESOURCE_STATE_COPY_SOURCE;
		case GFX::Resource::State::ResolveDestination:
			return D3D12_RESOURCE_STATE_RESOLVE_DEST;
		case GFX::Resource::State::ResolveSource:
			return D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
		case GFX::Resource::State::StreamOut:
			return D3D12_RESOURCE_STATE_STREAM_OUT;
		case GFX::Resource::State::Indirect:
			return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
		case GFX::Resource::State::Predication:
			return D3D12_RESOURCE_STATE_PREDICATION;
		case GFX::Resource::State::AccelerationStructureRT:
			return D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
		case GFX::Resource::State::ShadingRateSource:
			return D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
		case GFX::Resource::State::VideoDecodeRead:
			return D3D12_RESOURCE_STATE_VIDEO_DECODE_READ;
		case GFX::Resource::State::VideoDecodeWrite:
			return D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE;
		case GFX::Resource::State::VideoProcessRead:
			return D3D12_RESOURCE_STATE_VIDEO_PROCESS_READ;
		case GFX::Resource::State::VideoProcessWrite:
			return D3D12_RESOURCE_STATE_VIDEO_PROCESS_WRITE;
		case GFX::Resource::State::VideoEncodeRead:
			return D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ;
		case GFX::Resource::State::VideoEncodeWrite:
			return D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE;
		}
		return D3D12_RESOURCE_STATE_COMMON;
	}

	constexpr D3D12_SHADER_VISIBILITY GetShaderVisibility(GFX::Resource::ShaderType type, std::bitset<6>* presence) noexcept
	{
		switch (type)
		{
		case GFX::Resource::ShaderType::Vertex:
		{
			if (presence)
				(*presence)[0] = true;
			return D3D12_SHADER_VISIBILITY_VERTEX;
		}
		case GFX::Resource::ShaderType::Domain:
		{
			if (presence)
				(*presence)[1] = true;
			return D3D12_SHADER_VISIBILITY_DOMAIN;
		}
		case GFX::Resource::ShaderType::Hull:
		{
			if (presence)
				(*presence)[2] = true;
			return D3D12_SHADER_VISIBILITY_HULL;
		}
		case GFX::Resource::ShaderType::Geometry:
		{
			if (presence)
				(*presence)[3] = true;
			return D3D12_SHADER_VISIBILITY_GEOMETRY;
		}
		case GFX::Resource::ShaderType::Pixel:
		{
			if (presence)
				(*presence)[4] = true;
			return D3D12_SHADER_VISIBILITY_PIXEL;
		}
		}
		if (presence)
			(*presence)[5] = true;
		return D3D12_SHADER_VISIBILITY_ALL;
	}

	constexpr D3D12_STATIC_BORDER_COLOR GetStaticBorderColor(GFX::Resource::Texture::EdgeColor color) noexcept
	{
		switch (color)
		{
		case GFX::Resource::Texture::EdgeColor::SolidBlack:
			return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		case GFX::Resource::Texture::EdgeColor::SolidWhite:
			return D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		}
		return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	}

	constexpr D3D12_TEXTURE_ADDRESS_MODE GetTextureAddressMode(GFX::Resource::Texture::AddressMode mode) noexcept
	{
		switch (mode)
		{
		case GFX::Resource::Texture::AddressMode::Mirror:
			return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		case GFX::Resource::Texture::AddressMode::Edge:
			return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		case GFX::Resource::Texture::AddressMode::BorderColor:
			return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		case GFX::Resource::Texture::AddressMode::MirrorOnce:
			return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
		}
		return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	}

	constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE GetTopologyType(GFX::Resource::TopologyType type) noexcept
	{
		switch (type)
		{
		case GFX::Resource::TopologyType::Point:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		case GFX::Resource::TopologyType::Line:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case GFX::Resource::TopologyType::Triangle:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		case GFX::Resource::TopologyType::ControlPoint:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
		}
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
	}

	constexpr D3D12_RESOURCE_BARRIER_FLAGS GetTransitionType(GFX::Pipeline::BarrierType type) noexcept
	{
		switch (type)
		{
		case GFX::Pipeline::BarrierType::Begin:
			return D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
		case GFX::Pipeline::BarrierType::End:
			return D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
		}
		return D3D12_RESOURCE_BARRIER_FLAG_NONE;
	}
#pragma endregion
}