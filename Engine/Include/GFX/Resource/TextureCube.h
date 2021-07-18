#pragma once
#include "GFX/API/DX11/Resource/TextureCube.h"

namespace ZE::GFX::Resource
{
	// Cubic texture data for shaders
	class TextureCube final
	{
		ZE_API_BACKEND(TextureCube, DX11, DX11, DX11, DX11) backend;

	public:
		constexpr TextureCube(Device& dev, const std::string& dir, const std::string& ext);
		TextureCube(TextureCube&&) = delete;
		TextureCube(const TextureCube&) = delete;
		TextureCube& operator=(TextureCube&&) = delete;
		TextureCube& operator=(const TextureCube&) = delete;
		~TextureCube() = default;

		constexpr ZE_API_BACKEND(TextureCube, DX11, DX11, DX11, DX11)& Get() noexcept { return backend; }
		void BindPS(Context& ctx, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(backend, BindPS, ctx, slot); }
		void BindCS(Context& ctx, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(backend, BindCS, ctx, slot); }

		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, Context& ctx);
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
		backend.Init(dev, surfaces);
	}

	constexpr void TextureCube::SwitchApi(GfxApiType nextApi, Device& dev, Context& ctx)
	{
		std::array<Surface, 6> surfaces;
		ZE_API_BACKEND_CALL_RET(backend, surfaces, GetData, dev, ctx);
		backend.Switch(nextApi, dev, surfaces);
	}
#pragma endregion
}