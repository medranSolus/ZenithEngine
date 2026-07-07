#pragma once
#include "GFX/Resource/Texture/PackDesc.h"
#include "GFX/Binding/Context.h"
#include "GFX/CommandList.h"
#include "GFX/GFile.h"

namespace ZE::RHI::DX11::Resource::Texture
{
	class Pack final
	{
		U32 count = 0;
		std::unique_ptr<DX::ComPtr<IShaderResourceView>[]> srvs;

	public:
		Pack() = default;
		ZE_CLASS_MOVE(Pack);
		~Pack() = default;

		static Expected<Pack> Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::Texture::PackDesc& desc) noexcept;
		static Expected<Pack> Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::Texture::PackFileDesc& desc, GFX::GFile& file) noexcept;

		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;

		// Gfx API Internal

		DX::ComPtr<IShaderResourceView> GetView(U32 index) const noexcept { ZE_ASSERT(index < count, "Texture resource index out of range!"); return srvs[index]; }
	};
}