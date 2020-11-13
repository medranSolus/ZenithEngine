#pragma once
#include "GfxResPtr.h"

namespace GFX::Resource
{
	class Topology : public IBindable
	{
		D3D11_PRIMITIVE_TOPOLOGY type;

	public:
		constexpr Topology(Graphics& gfx, D3D11_PRIMITIVE_TOPOLOGY type) : type(type) {}
		virtual ~Topology() = default;

		static inline GfxResPtr<Topology> Get(Graphics& gfx, D3D11_PRIMITIVE_TOPOLOGY type) { return Codex::Resolve<Topology>(gfx, type); }
		static inline std::string GenerateRID(D3D11_PRIMITIVE_TOPOLOGY type) noexcept { return "T" + std::to_string(type); }

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->IASetPrimitiveTopology(type); }
		inline std::string GetRID() const noexcept override { return GenerateRID(type); }
	};

	template<>
	struct is_resolvable_by_codex<Topology>
	{
		static constexpr bool generate{ true };
	};
}