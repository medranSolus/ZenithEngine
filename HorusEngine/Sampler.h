#pragma once
#include "Codex.h"

namespace GFX::Resource
{
	// How to perform lookups into texture
	class Sampler : public IBindable
	{
	public:
		enum Type : unsigned char { Point, Linear, Anisotropic };

	private:
		Type type;
		bool textureCoordReflect;
		UINT slot;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> state;

	public:
		Sampler(Graphics& gfx, Type type, bool textureCoordReflect, UINT slot = 0U);
		inline Sampler(Sampler&& sampler) noexcept { *this = std::forward<Sampler&&>(sampler); }
		Sampler& operator=(Sampler&& sampler) noexcept;
		virtual ~Sampler() = default;

		static inline std::shared_ptr<Sampler> Get(Graphics& gfx, Type type, bool textureCoordReflect, UINT slot = 0U);
		static inline std::string GenerateRID(Type type, bool textureCoordReflect, UINT slot = 0U) noexcept;

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->PSSetSamplers(slot, 1U, state.GetAddressOf()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(type, textureCoordReflect, slot); }
	};

	template<>
	struct is_resolvable_by_codex<Sampler>
	{
		static constexpr bool generate{ true };
	};

	inline std::shared_ptr<Sampler> Sampler::Get(Graphics& gfx, Type type, bool textureCoordReflect, UINT slot)
	{
		return Codex::Resolve<Sampler>(gfx, type, textureCoordReflect, slot);
	}

	inline std::string Sampler::GenerateRID(Type type, bool textureCoordReflect, UINT slot) noexcept
	{
		return "#" + std::string(typeid(Sampler).name()) + "#" + std::to_string(type) + "#" + std::to_string(textureCoordReflect) + "#" + std::to_string(slot) + "#";
	}
}