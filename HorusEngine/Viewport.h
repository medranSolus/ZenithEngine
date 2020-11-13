#pragma once
#include "GfxResPtr.h"

namespace GFX::Resource
{
	class Viewport : public IBindable
	{
		D3D11_VIEWPORT viewport;
		unsigned int width;
		unsigned int height;

	public:
		constexpr Viewport(Graphics& gfx, unsigned int width, unsigned int height) noexcept;
		virtual ~Viewport() = default;

		static inline GfxResPtr<Viewport> Get(Graphics& gfx, unsigned int width, unsigned int height) { return Codex::Resolve<Viewport>(gfx, width, height); }
		static inline std::string GenerateRID(unsigned int width, unsigned int height) noexcept;

		constexpr unsigned int GetWidth() const noexcept { return width; }
		constexpr unsigned int GetHeight() const noexcept { return height; }

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->RSSetViewports(1U, &viewport); }
		inline std::string GetRID() const noexcept override { return GenerateRID(width, height); }
	};

	template<>
	struct is_resolvable_by_codex<Viewport>
	{
		static constexpr bool generate{ true };
	};

	constexpr Viewport::Viewport(Graphics& gfx, unsigned int width, unsigned int height) noexcept
		: width(width), height(height)
	{
		viewport.Width = static_cast<FLOAT>(width);
		viewport.Height = static_cast<FLOAT>(height);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
	}

	inline std::string Viewport::GenerateRID(unsigned int width, unsigned int height) noexcept
	{
		return "V" + std::to_string(width) + "#" + std::to_string(height);
	}
}