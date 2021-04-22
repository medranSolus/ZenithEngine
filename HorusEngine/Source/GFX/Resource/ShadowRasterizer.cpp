#include "GFX/Resource/ShadowRasterizer.h"
#include "GFX/Resource/GfxDebugName.h"
#include "GFX/Graphics.h"

namespace GFX::Resource
{
	ShadowRasterizer::ShadowRasterizer(Graphics& gfx, S32 depthBias, float slopeBias, float biasClamp)
		: depthBias(depthBias), slopeBias(slopeBias), biasClamp(biasClamp)
	{
		GFX_ENABLE_ALL(gfx);

		D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
		rasterDesc.DepthBias = depthBias; // Base bias for every pixel (minimal for floating point error)
		rasterDesc.SlopeScaledDepthBias = slopeBias; // Bias * level of slope
		rasterDesc.DepthBiasClamp = biasClamp; // Max slope bias
		GFX_THROW_FAILED(GetDevice(gfx)->CreateRasterizerState(&rasterDesc, &state));
		GFX_SET_ID(state.Get(), "SR");
	}

	GfxResPtr<ShadowRasterizer> ShadowRasterizer::Get(Graphics& gfx, S32 depthBias, float slopeBias, float biasClamp)
	{
		return GfxResPtr<ShadowRasterizer>(gfx, depthBias, slopeBias, biasClamp);
	}
}