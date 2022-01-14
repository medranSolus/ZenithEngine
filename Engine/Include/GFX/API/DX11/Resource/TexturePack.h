#pragma once
#include "GFX/Device.h"
#include "GFX/Binding/Context.h"
#include "GFX/Resource/TexturePackDesc.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11::Resource
{
	class TexturePack final
	{
		U32 count = 0;
		DX::ComPtr<ID3D11ShaderResourceView>* srvs = nullptr;

	public:
		TexturePack(GFX::Device& dev, const GFX::Resource::TexturePackDesc& desc);
		ZE_CLASS_MOVE(TexturePack);
		~TexturePack();

		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;
		std::vector<std::vector<Surface>> GetData(GFX::Device& dev) const;
	};
}