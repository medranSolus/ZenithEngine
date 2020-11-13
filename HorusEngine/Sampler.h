#pragma once
#include "GfxResPtr.h"

namespace GFX::Resource
{
	// How to perform lookups into texture
	class Sampler : public IBindable
	{
	public:
		enum Type : uint8_t { Point, Linear, Anisotropic };
		enum CoordType : uint8_t { Wrap, Reflect, Border };

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

		static inline GfxResPtr<Sampler> Get(Graphics& gfx, Type type, CoordType coordType, UINT slot = 0U);
		static inline std::string GenerateRID(Type type, CoordType coordType, UINT slot = 0U) noexcept;

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->PSSetSamplers(slot, 1U, state.GetAddressOf()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(type, coordType, slot); }
	};

	template<>
	struct is_resolvable_by_codex<Sampler>
	{
		static constexpr bool generate{ true };
	};

	inline GfxResPtr<Sampler> Sampler::Get(Graphics& gfx, Type type, CoordType coordType, UINT slot)
	{
		return Codex::Resolve<Sampler>(gfx, type, coordType, slot);
	}

	inline std::string Sampler::GenerateRID(Type type, CoordType coordType, UINT slot) noexcept
	{
		return "S" + std::to_string(type) + "#" + std::to_string(coordType) + "#" + std::to_string(slot);
	}
}