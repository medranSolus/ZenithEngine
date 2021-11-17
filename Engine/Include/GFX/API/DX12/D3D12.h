#pragma once
// Headers needed for DirectX 12
#include "GFX/API/DX/DXGI.h"
#include "GFX/Pipeline/BarrierType.h"
#include "GFX/Resource/PipelineStateDesc.h"
#include "GFX/Resource/State.h"
#include "GFX/Resource/Topology.h"
#include "GFX/CommandType.h"
#include <d3d12.h>

namespace ZE::GFX::API::DX12
{
	// Get DirectX 12 version of command list types
	constexpr D3D12_COMMAND_LIST_TYPE GetCommandType(CommandType type) noexcept;
	// Get DirectX 12 version of possible barrier types
	constexpr D3D12_RESOURCE_BARRIER_FLAGS GetTransitionType(GFX::Pipeline::BarrierType type) noexcept;
	// Get DirectX 12 version of resource states
	constexpr D3D12_RESOURCE_STATES GetResourceState(GFX::Resource::State state) noexcept;
	// Get DirectX 12 version of primitive topology types
	constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE GetTopologyType(GFX::Resource::TopologyType type) noexcept;
	// Get DirectX 12 version of culling modes
	constexpr D3D12_CULL_MODE GetCulling(GFX::Resource::CullMode mode) noexcept;

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
#pragma endregion
}