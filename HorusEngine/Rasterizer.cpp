#include "Rasterizer.h"
#include "GfxExceptionMacros.h"

namespace GFX::Resource
{
	Rasterizer::Rasterizer(Graphics& gfx, D3D11_CULL_MODE culling, bool depthEnable)
		: culling(culling), depthEnable(depthEnable)
	{
		GFX_ENABLE_ALL(gfx);

		D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
		rasterDesc.CullMode = culling;
		rasterDesc.DepthClipEnable = depthEnable;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateRasterizerState(&rasterDesc, &state));
		SET_DEBUG_NAME_RID(state.Get());
	}
}