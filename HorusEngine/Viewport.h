#pragma once
#include "Codex.h"

namespace GFX::Resource
{
	class Viewport : public IBindable
	{
		D3D11_VIEWPORT viewport;
		unsigned int width;
		unsigned int height;

	public:
		Viewport(Graphics& gfx, unsigned int width, unsigned int height);
		virtual ~Viewport() = default;

		static inline std::shared_ptr<Viewport> Get(Graphics& gfx, unsigned int width, unsigned int height) { return Codex::Resolve<Viewport>(gfx, width, height); }
		static inline std::string GenerateRID(unsigned int width, unsigned int height) noexcept;

		constexpr unsigned int GetWidth() const noexcept { return width; }
		constexpr unsigned int GetHeight() const noexcept { return height; }

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->RSSetViewports(1U, &viewport); }
		inline std::string GetRID() const noexcept override { return GenerateRID(width, height); }
	};

	template<>
	struct is_resolvable_by_codex<Viewport>
	{
		static constexpr bool generate{ true };
	};

	inline std::string Viewport::GenerateRID(unsigned int width, unsigned int height) noexcept
	{
		return "#" + std::string(typeid(Viewport).name()) + "#" + std::to_string(width) + "#" + std::to_string(height) + "#";
	}
}