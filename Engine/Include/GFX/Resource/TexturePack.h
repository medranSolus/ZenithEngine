#pragma once
#include "GFX/API/DX11/Resource/TexturePack.h"
#include "GFX/API/DX12/Resource/TexturePack.h"

namespace ZE::GFX::Resource
{
	// Class containing textures
	class TexturePack final
	{
		ZE_API_BACKEND(Resource::TexturePack);

	public:
		TexturePack() = default;
		constexpr TexturePack(Device& dev, const TexturePackDesc& desc) { ZE_API_BACKEND_VAR.Init(dev, desc); }
		ZE_CLASS_MOVE(TexturePack);
		~TexturePack() = default;

		constexpr void Init(Device& dev, const TexturePackDesc& desc) { ZE_API_BACKEND_VAR.Init(dev, desc); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, const TexturePackDesc& desc) { ZE_API_BACKEND_VAR.Switch(nextApi, dev, desc); }
		ZE_API_BACKEND_GET(Resource::TexturePack);

		// Main Gfx API

		constexpr void Bind(CommandList& cl, Binding::Context& bindCtx) const { ZE_API_BACKEND_CALL(Bind, cl, bindCtx); }
	};
}