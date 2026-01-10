#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/Resource/Texture/Pack.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/Resource/Texture/Pack.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/Resource/Texture/Pack.h"
#endif

namespace ZE::GFX::Resource::Texture
{
	// Class containing textures
	class Pack final
	{
		ZE_RHI_BACKEND(Resource::Texture::Pack);

	public:
		Pack() = default;
		constexpr Pack(Device& dev, DiskManager& disk, const PackDesc& desc) { Init(dev, disk, desc); }
		constexpr Pack(Device& dev, DiskManager& disk, const PackFileDesc& desc, GFile& file) { Init(dev, disk, desc, file); }
		ZE_CLASS_MOVE(Pack);
		~Pack() = default;

		constexpr void Init(Device& dev, DiskManager& disk, const PackDesc& desc) { ZE_ASSERT(desc.Textures.size() > 0, "Cannot create empty texture pack!"); ZE_RHI_BACKEND_VAR.Init(dev, disk, desc); }
		constexpr void Init(Device& dev, DiskManager& disk, const PackFileDesc& desc, GFile& file) { ZE_ASSERT(desc.Textures.size() > 0, "Cannot create empty texture pack!"); ZE_RHI_BACKEND_VAR.Init(dev, disk, desc, file); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, DiskManager& disk, const PackDesc& desc) { ZE_RHI_BACKEND_VAR.Switch(nextApi, dev, disk, desc); }
		ZE_RHI_BACKEND_GET(Resource::Texture::Pack);

		// Main Gfx API

		constexpr void Bind(CommandList& cl, Binding::Context& bindCtx) const { ZE_RHI_BACKEND_CALL(Bind, cl, bindCtx); }
		// Before destroying texture pack you have to call this function for proper memory freeing
		constexpr void Free(Device& dev) noexcept { ZE_RHI_BACKEND_CALL(Free, dev); }
	};
}