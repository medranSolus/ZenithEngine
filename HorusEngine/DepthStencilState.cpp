#include "DepthStencilState.h"

namespace GFX::Resource
{
	DepthStencilState::DepthStencilState(Graphics& gfx, StencilMode mode) : mode(mode)
	{
		D3D11_DEPTH_STENCIL_DESC desc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };
		switch (mode)
		{
		case GFX::Resource::DepthStencilState::Write:
		{
			desc.DepthEnable = FALSE;
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ZERO;
			desc.StencilEnable = TRUE;
			desc.StencilWriteMask = 0xFF;
			desc.FrontFace.StencilFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;
			desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_REPLACE;
			break;
		}
		case GFX::Resource::DepthStencilState::Mask:
		{
			desc.DepthEnable = FALSE;
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ZERO;
			desc.StencilEnable = TRUE;
			desc.StencilReadMask = 0xFF;
			desc.FrontFace.StencilFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NOT_EQUAL;
			desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
			break;
		}
		}
		GetDevice(gfx)->CreateDepthStencilState(&desc, &state);
	}
}