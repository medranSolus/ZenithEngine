#pragma once
#include "GFX/API/DX11/Resource/TextureCube.h"

namespace ZE::GFX::Resource
{
	// Cubic texture data for shaders
	class TextureCube final
	{
		ZE_API_BACKEND(TextureCube);

	public:
		constexpr TextureCube(Device& dev, const std::string& dir, const std::string& ext);
		ZE_CLASS_MOVE(TextureCube);
		~TextureCube() = default;

		ZE_API_BACKEND_GET(TextureCube);
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, Context& ctx);

		// Main Gfx API

		constexpr void BindPS(Context& ctx, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(BindPS, ctx, slot); }
		constexpr void BindCS(Context& ctx, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(BindCS, ctx, slot); }
	};

#pragma region Functions
	constexpr TextureCube::TextureCube(Device& dev, const std::string& dir, const std::string& ext)
	{
		std::array<Surface, 6> surfaces =
		{
			dir + "/px" + ext, // Right
			dir + "/nx" + ext, // Left
			dir + "/py" + ext, // Up
			dir + "/ny" + ext, // Down
			dir + "/pz" + ext, // Front
			dir + "/nz" + ext  // Back
		}
		ZE_API_BACKEND_VAR.Init(dev, surfaces);
	}

	constexpr void TextureCube::SwitchApi(GfxApiType nextApi, Device& dev, Context& ctx)
	{
		std::array<Surface, 6> surfaces;
		ZE_API_BACKEND_CALL_RET(surfaces, GetData, dev, ctx);
		ZE_API_BACKEND_VAR.Switch(nextApi, dev, surfaces);
	}
#pragma endregion
}