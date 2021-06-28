#pragma once
#include "GfxResPtr.h"

namespace ZE::GFX::Resource
{
	class Topology : public IBindable
	{
		D3D11_PRIMITIVE_TOPOLOGY type;

	public:
		constexpr Topology(Graphics& gfx, D3D11_PRIMITIVE_TOPOLOGY type) : type(type) {}
		virtual ~Topology() = default;

		static std::string GenerateRID(D3D11_PRIMITIVE_TOPOLOGY type) noexcept { return "T" + std::to_string(type); }
		static GfxResPtr<Topology> Get(Graphics& gfx, D3D11_PRIMITIVE_TOPOLOGY type) { return Codex::Resolve<Topology>(gfx, type); }

		void Bind(Graphics& gfx) const override { GetContext(gfx)->IASetPrimitiveTopology(type); }
		std::string GetRID() const noexcept override { return GenerateRID(type); }
	};

	template<>
	struct is_resolvable_by_codex<Topology>
	{
		static constexpr bool GENERATE_ID{ true };
	};
}