#pragma once
#include "Codex.h"

namespace GFX::Resource
{
	// How to perform lookups into texture
	class Sampler : public IBindable
	{
	public:
		enum Type : unsigned char { Point, Linear, Anisotropic };
		enum CoordType : unsigned char { Wrap, Reflect, Border };

	private:
		Type type;
		CoordType coordType;
		UINT slot;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> state;

	public:
		Sampler(Graphics& gfx, Type type, CoordType coordType, UINT slot = 0U);
		inline Sampler(Sampler&& sampler) noexcept { *this = std::forward<Sampler&&>(sampler); }
		Sampler& operator=(Sampler&& sampler) noexcept;
		virtual ~Sampler() = default;

		static inline std::shared_ptr<Sampler> Get(Graphics& gfx, Type type, CoordType coordType, UINT slot = 0U);
		static inline std::string GenerateRID(Type type, CoordType coordType, UINT slot = 0U) noexcept;

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->PSSetSamplers(slot, 1U, state.GetAddressOf()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(type, coordType, slot); }
	};

	template<>
	struct is_resolvable_by_codex<Sampler>
	{
		static constexpr bool generate{ true };
	};

	inline std::shared_ptr<Sampler> Sampler::Get(Graphics& gfx, Type type, CoordType coordType, UINT slot)
	{
		return Codex::Resolve<Sampler>(gfx, type, coordType, slot);
	}

	inline std::string Sampler::GenerateRID(Type type, CoordType coordType, UINT slot) noexcept
	{
		return "#" + std::string(typeid(Sampler).name()) + "#" + std::to_string(type) + "#" + std::to_string(coordType) + "#" + std::to_string(slot) + "#";
	}
}