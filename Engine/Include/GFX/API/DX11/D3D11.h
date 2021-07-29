#pragma once
// Headers needed for DirectX 11
#include "GFX/API/DX/DXGI.h"
#include "GFX/Resource/PipelineStateDesc.h"
#include <d3d11_4.h>

namespace ZE::GFX::API::DX11
{
	constexpr D3D11_CULL_MODE GetCulling(Resource::CullMode mode) noexcept
	{
		switch (mode)
		{
		case Resource::CullMode::Front:
			return D3D11_CULL_FRONT;
		case Resource::CullMode::Back:
			return D3D11_CULL_BACK;
		}
		return D3D11_CULL_NONE;
	}
}