#pragma once
#include "GfxResPtr.h"

namespace ZE::GFX::Resource
{
	// How to perform lookups into texture
	class Sampler : public IBindable
	{
	public:
		enum Type : U8 { Point, Linear, Anisotropic };
		enum CoordType : U8 { Wrap, Reflect, Border };

	private:
		Microsoft::WRL::ComPtr<ID3D11SamplerState> state;
		U32 slot;
		Type type;
		CoordType coordType;

	public:
		Sampler(Graphics& gfx, Type type, CoordType coordType, U32 slot = 0);
		Sampler(const Sampler& s) noexcept { *this = s; }
		Sampler& operator=(const Sampler& s) noexcept;
		Sampler(Sampler&&) = default;
		Sampler& operator=(Sampler&&) = default;
		virtual ~Sampler() = default;

		static std::string GenerateRID(Type type, CoordType coordType, U32 slot = 0) noexcept;
		static GfxResPtr<Sampler> Get(Graphics& gfx, Type type, CoordType coordType, U32 slot = 0);

		void Bind(Graphics& gfx) const override { GetContext(gfx)->PSSetSamplers(slot, 1, state.GetAddressOf()); }
		std::string GetRID() const noexcept override { return GenerateRID(type, coordType, slot); }
	};

	template<>
	struct is_resolvable_by_codex<Sampler>
	{
		static constexpr bool GENERATE_ID{ true };
	};
}