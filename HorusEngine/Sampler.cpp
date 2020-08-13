#include "Sampler.h"
#include "GfxExceptionMacros.h"

namespace GFX::Resource
{
	Sampler::Sampler(Graphics& gfx, Type type, bool textureCoordReflect) : type(type), textureCoordReflect(textureCoordReflect)
	{
		GFX_ENABLE_ALL(gfx);

		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT{});
		switch (type)
		{
		case GFX::Resource::Sampler::Point:
		{
			samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_POINT;
			break;
		}
		case GFX::Resource::Sampler::Linear:
		{
			samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		}
		case GFX::Resource::Sampler::Anisotropic:
		{
			samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_ANISOTROPIC;
			break;
		}
		}
		samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
		samplerDesc.AddressV = samplerDesc.AddressU =
			textureCoordReflect ? D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_MIRROR : D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateSamplerState(&samplerDesc, &state));
	}
}