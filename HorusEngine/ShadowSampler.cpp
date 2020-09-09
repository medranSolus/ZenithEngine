#include "ShadowSampler.h"
#include "GfxExceptionMacros.h"

namespace GFX::Resource
{
	ShadowSampler::ShadowSampler(Graphics& gfx)
	{
		GFX_ENABLE_ALL(gfx);

		// When sampling outside texture give constant color
		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT{});
		samplerDesc.BorderColor[0] = 1.0f;
		samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
		samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_BORDER;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateSamplerState(&samplerDesc, &state));
	}
}