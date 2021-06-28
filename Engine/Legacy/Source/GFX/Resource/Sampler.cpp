#include "GFX/Resource/Sampler.h"
#include "GFX/Resource/GfxDebugName.h"
#include "GFX/Graphics.h"

namespace ZE::GFX::Resource
{
	Sampler::Sampler(Graphics& gfx, Type type, CoordType coordType, U32 slot)
		: type(type), coordType(coordType), slot(slot)
	{
		ZE_GFX_ENABLE_ALL(gfx);

		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT{});
		samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
		switch (type)
		{
		case Type::Point:
		{
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			break;
		}
		case Type::Linear:
		{
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		}
		case Type::Anisotropic:
		{
			samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			break;
		}
		}
		switch (coordType)
		{
		case CoordType::Wrap:
		{
			samplerDesc.AddressV = samplerDesc.AddressU = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			break;
		}
		case CoordType::Reflect:
		{
			samplerDesc.AddressV = samplerDesc.AddressU = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
			break;
		}
		case CoordType::Border:
		{
			samplerDesc.AddressV = samplerDesc.AddressU = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.BorderColor[0] = FLT_MAX;
			break;
		}
		}
		ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateSamplerState(&samplerDesc, &state));
		ZE_GFX_SET_RID(state.Get());
	}

	Sampler& Sampler::operator=(const Sampler& s) noexcept
	{
		state = s.state;
		slot = s.slot;
		type = s.type;
		coordType = s.coordType;
		return *this;
	}

	std::string Sampler::GenerateRID(Type type, CoordType coordType, U32 slot) noexcept
	{
		return "S" + std::to_string(type) + std::to_string(coordType) + std::to_string(slot);
	}

	GfxResPtr<Sampler> Sampler::Get(Graphics& gfx, Type type, CoordType coordType, U32 slot)
	{
		return Codex::Resolve<Sampler>(gfx, type, coordType, slot);
	}
}