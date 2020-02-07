#include "Sampler.h"
#include "GfxExceptionMacros.h"

namespace GFX::Resource
{
	Sampler::Sampler(Graphics& gfx)
	{
		GFX_ENABLE_ALL(gfx);

		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT{});
		samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_ANISOTROPIC;
		samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateSamplerState(&samplerDesc, &state));
	}
}