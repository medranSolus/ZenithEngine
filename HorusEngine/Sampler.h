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
		Microsoft::WRL::ComPtr<ID3D11SamplerState> state;

	public:
		Sampler(Graphics& gfx, Type type, bool textureCoordReflect);
		virtual ~Sampler() = default;

		static inline std::shared_ptr<Sampler> Get(Graphics& gfx, Type type, bool textureCoordReflect);
		static inline std::string GenerateRID(Type type, bool textureCoordReflect) noexcept;

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->PSSetSamplers(0U, 1U, state.GetAddressOf()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(type, textureCoordReflect); }
	};

	template<>
	struct is_resolvable_by_codex<Sampler>
	{
		static constexpr bool generate{ true };
	};

	inline std::shared_ptr<Sampler> Sampler::Get(Graphics& gfx, Type type, bool textureCoordReflect)
	{
		return Codex::Resolve<Sampler>(gfx, type, textureCoordReflect);
	}

	inline std::string Sampler::GenerateRID(Type type, bool textureCoordReflect) noexcept
	{
		return "#" + std::string(typeid(Sampler).name()) + "#" + std::to_string(type) + "#" + std::to_string(textureCoordReflect) + "#";
	}
}