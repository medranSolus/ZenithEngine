#include "GFX/Resource/Rasterizer.h"
#include "GFX/Resource/GfxDebugName.h"
#include "GFX/Graphics.h"

namespace ZE::GFX::Resource
{
	Rasterizer::Rasterizer(Graphics& gfx, D3D11_CULL_MODE culling, bool depthEnable)
		: culling(culling), depthEnable(depthEnable)
	{
		ZE_GFX_ENABLE_ALL(gfx);

		D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
		rasterDesc.CullMode = culling;
		rasterDesc.DepthClipEnable = depthEnable;
		ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateRasterizerState(&rasterDesc, &state));
		ZE_GFX_SET_RID(state.Get());
	}

	std::string Rasterizer::GenerateRID(D3D11_CULL_MODE culling, bool depthEnable) noexcept
	{
		return "R" + std::to_string(depthEnable) + std::to_string(culling);
	}

	GfxResPtr<Rasterizer> Rasterizer::Get(Graphics& gfx, D3D11_CULL_MODE culling, bool depthEnable)
	{
		return Codex::Resolve<Rasterizer>(gfx, culling, depthEnable);
	}
}