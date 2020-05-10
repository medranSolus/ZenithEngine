#pragma once
#include "Codex.h"

namespace GFX::Resource
{
	class Topology : public IBindable
	{
	protected:
		D3D11_PRIMITIVE_TOPOLOGY type;

	public:
		constexpr Topology(Graphics& gfx, D3D11_PRIMITIVE_TOPOLOGY type) : type(type) {}
		Topology(const Topology&) = default;
		Topology& operator=(const Topology&) = default;
		virtual ~Topology() = default;

		static inline std::shared_ptr<Topology> Get(Graphics& gfx, D3D11_PRIMITIVE_TOPOLOGY type) { return Codex::Resolve<Topology>(gfx, type); }
		static inline std::string GenerateRID(D3D11_PRIMITIVE_TOPOLOGY type) noexcept { return "#" + std::string(typeid(Topology).name()) + "#" + std::to_string(type) + "#"; }

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->IASetPrimitiveTopology(type); }
		inline std::string GetRID() const noexcept override { return GenerateRID(type); }
	};

	template<>
	struct is_resolvable_by_codex<Topology>
	{
		static constexpr bool generate{ true };
	};
}