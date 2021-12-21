#pragma once
#include "GFX/API/DX11/Resource/Texture.h"

namespace ZE::GFX::Resource
{
	// Texture data for shaders
	class Texture final
	{
		ZE_API_BACKEND(Texture);

	public:
		constexpr Texture(Device& dev, Context& ctx, const Surface& surface) { ZE_API_BACKEND_VAR.Init(dev, ctx, surface); }
		ZE_CLASS_MOVE(Texture);
		~Texture() = default;

		ZE_API_BACKEND_GET(Texture);
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, Context& ctx);

		// Main Gfx API

		constexpr void BindPS(Context& ctx, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(BindPS, ctx, slot); }
		constexpr void BindCS(Context& ctx, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(BindCS, ctx, slot); }
	};

#pragma region Functions
	constexpr void Texture::SwitchApi(GfxApiType nextApi, Device& dev, Context& ctx)
	{
		Surface surface;
		ZE_API_BACKEND_CALL_RET(surface, GetData, dev, ctx);
		ZE_API_BACKEND_VAR.Switch(nextApi, dev, surface);
	}
#pragma endregion
}