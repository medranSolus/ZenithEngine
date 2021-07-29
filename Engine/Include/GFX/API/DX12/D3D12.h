#pragma once
// Headers needed for DirectX 12
#include "GFX/API/DX/DXGI.h"
#include "GFX/Resource/PipelineStateDesc.h"
#include <d3d12.h>

namespace ZE::GFX::API::DX12
{
	constexpr D3D12_CULL_MODE GetCulling(Resource::CullMode mode) noexcept
	{
		switch (mode)
		{
		case Resource::CullMode::Front:
			return D3D12_CULL_MODE_FRONT;
		case Resource::CullMode::Back:
			return D3D12_CULL_MODE_BACK;
		}
		return D3D12_CULL_MODE_NONE;
	}
}