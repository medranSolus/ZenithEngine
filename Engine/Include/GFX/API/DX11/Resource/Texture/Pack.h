#pragma once
#include "GFX/Device.h"
#include "GFX/Binding/Context.h"
#include "GFX/Resource/Texture/PackDesc.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11::Resource::Texture
{
	class Pack final
	{
		U32 count = 0;
		DX::ComPtr<ID3D11ShaderResourceView>* srvs = nullptr;

	public:
		Pack(GFX::Device& dev, const GFX::Resource::Texture::PackDesc& desc);
		ZE_CLASS_MOVE(Pack);
		~Pack();

		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;
		std::vector<std::vector<Surface>> GetData(GFX::Device& dev) const;
	};
}