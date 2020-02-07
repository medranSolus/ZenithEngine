#include "Rasterizer.h"
#include "GfxExceptionMacros.h"

namespace GFX::Resource
{
	Rasterizer::Rasterizer(Graphics& gfx, bool culling) : culling(culling)
	{
		GFX_ENABLE_ALL(gfx);

		D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
		rasterDesc.CullMode = culling ? D3D11_CULL_MODE::D3D11_CULL_BACK : D3D11_CULL_MODE::D3D11_CULL_NONE;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateRasterizerState(&rasterDesc, &state));
	}
}