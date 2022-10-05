#pragma once
#include "GFX/API/DX11/Resource/Texture/Pack.h"
#include "GFX/API/DX12/Resource/Texture/Pack.h"
#include "GFX/API/VK/Resource/Texture/Pack.h"

namespace ZE::GFX::Resource::Texture
{
	// Class containing textures
	class Pack final
	{
		ZE_API_BACKEND(Resource::Texture::Pack);

	public:
		Pack() = default;
		constexpr Pack(Device& dev, const PackDesc& desc) { ZE_API_BACKEND_VAR.Init(dev, desc); }
		ZE_CLASS_MOVE(Pack);
		~Pack() = default;

		constexpr void Init(Device& dev, const PackDesc& desc) { ZE_API_BACKEND_VAR.Init(dev, desc); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, const PackDesc& desc) { ZE_API_BACKEND_VAR.Switch(nextApi, dev, desc); }
		ZE_API_BACKEND_GET(Resource::Texture::Pack);

		// Main Gfx API

		constexpr void Bind(CommandList& cl, Binding::Context& bindCtx) const { ZE_API_BACKEND_CALL(Bind, cl, bindCtx); }
		// Before destroying texture pack you have to call this function for proper memory freeing
		constexpr void Free(Device& dev) noexcept { ZE_API_BACKEND_CALL(Free, dev); }
	};
}