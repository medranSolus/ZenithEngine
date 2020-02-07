#include "Blender.h"
#include "GfxExceptionMacros.h"

namespace GFX::Resource
{
	Blender::Blender(Graphics& gfx, bool enabled) : enabled(enabled)
	{
		GFX_ENABLE_ALL(gfx);

		D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC(CD3D11_DEFAULT{});
		auto& blendTarget = blendDesc.RenderTarget[0];
		if (enabled)
		{
			blendTarget.BlendEnable = TRUE;
			blendTarget.SrcBlend = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
			blendTarget.DestBlend = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
		}
		GFX_THROW_FAILED(GetDevice(gfx)->CreateBlendState(&blendDesc, &state));
	}
}