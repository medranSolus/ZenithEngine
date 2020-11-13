#include "Sampler.h"
#include "GfxExceptionMacros.h"

namespace GFX::Resource
{
	Sampler::Sampler(Graphics& gfx, Type type, CoordType coordType, UINT slot)
		: type(type), coordType(coordType), slot(slot)
	{
		GFX_ENABLE_ALL(gfx);

		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT{});
		samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
		switch (type)
		{
		case Type::Point:
		{
			samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_POINT;
			break;
		}
		case Type::Linear:
		{
			samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		}
		case Type::Anisotropic:
		{
			samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_ANISOTROPIC;
			break;
		}
		}
		switch (coordType)
		{
		case CoordType::Wrap:
		{
			samplerDesc.AddressV = samplerDesc.AddressU = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
			break;
		}
		case CoordType::Reflect:
		{
			samplerDesc.AddressV = samplerDesc.AddressU = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_MIRROR;
			break;
		}
		case CoordType::Border:
		{
			samplerDesc.AddressV = samplerDesc.AddressU = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.BorderColor[0] = FLT_MAX;
			break;
		}
		}
		GFX_THROW_FAILED(GetDevice(gfx)->CreateSamplerState(&samplerDesc, &state));
		SET_DEBUG_NAME_RID(state.Get());
	}

	Sampler& Sampler::operator=(Sampler&& sampler) noexcept
	{
		type = sampler.type;
		coordType = sampler.coordType;
		slot = sampler.slot;
		state = std::move(sampler.state);
		return *this;
	}
}