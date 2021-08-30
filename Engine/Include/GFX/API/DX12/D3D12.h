#pragma once
// Headers needed for DirectX 12
#include "GFX/API/DX/DXGI.h"
#include "GFX/Resource/PipelineStateDesc.h"
#include "GFX/Resource/Topology.h"
#include "GFX/CommandType.h"
#include <d3d12.h>

namespace ZE::GFX::API::DX12
{
	constexpr D3D12_COMMAND_LIST_TYPE GetCommandType(CommandType type) noexcept;
	constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE GetTopologyType(GFX::Resource::TopologyType type) noexcept;
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