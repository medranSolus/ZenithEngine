#pragma once
// Headers needed for DirectX 11
#include "GFX/API/DX/DXGI.h"
#include "GFX/Resource/PipelineStateDesc.h"
#include "WarningGuardOn.h"
#include <d3d11_4.h>
#include "WarningGuardOff.h"

namespace ZE::GFX::API::DX11
{
	// Get DirectX 11 version of culling modes
	constexpr D3D11_CULL_MODE GetCulling(GFX::Resource::CullMode mode) noexcept
	{
		switch (mode)
		{
		default:
			ZE_ASSERT(false, "Unhandled enum value!");
		case GFX::Resource::CullMode::None:
			return D3D11_CULL_NONE;
		case GFX::Resource::CullMode::Front:
			return D3D11_CULL_FRONT;
		case GFX::Resource::CullMode::Back:
			return D3D11_CULL_BACK;
		}
	}
}