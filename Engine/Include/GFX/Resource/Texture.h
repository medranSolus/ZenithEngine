#pragma once
#include "GFX/API/DX11/Resource/Texture.h"

namespace ZE::GFX::Resource
{
	// Texture data for shaders
	class Texture final
	{
		ZE_API_BACKEND(Texture) backend;

	public:
		constexpr Texture(Device& dev, Context& ctx, const Surface& surface) { backend.Init(dev, ctx, surface); }
		Texture(Texture&&) = default;
		Texture(const Texture&) = delete;
		Texture& operator=(Texture&&) = default;
		Texture& operator=(const Texture&) = delete;
		~Texture() = default;

		constexpr ZE_API_BACKEND(Texture)& Get() noexcept { return backend; }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, Context& ctx);

		// Main Gfx API

		constexpr void BindPS(Context& ctx, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(backend, BindPS, ctx, slot); }
		constexpr void BindCS(Context& ctx, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(backend, BindCS, ctx, slot); }
	};

#pragma region Functions
	constexpr void Texture::SwitchApi(GfxApiType nextApi, Device& dev, Context& ctx)
	{
		Surface surface;
		ZE_API_BACKEND_CALL_RET(backend, surface, GetData, dev, ctx);
		backend.Switch(nextApi, dev, surface);
	}
#pragma endregion
}