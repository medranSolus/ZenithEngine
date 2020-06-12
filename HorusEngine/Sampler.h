#pragma once
#include "Codex.h"

namespace GFX::Resource
{
	// How to perform lookups into texture
	class Sampler : public IBindable
	{
		bool anisotropic;
		bool textureCoordWrap;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> state;

	public:
		Sampler(Graphics& gfx, bool anisotropic, bool textureCoordWrap);
		Sampler(const Sampler&) = delete;
		Sampler& operator=(const Sampler&) = delete;
		virtual ~Sampler() = default;

		static inline std::shared_ptr<Sampler> Get(Graphics& gfx, bool anisotropic = true, bool textureCoordWrap = false);
		static inline std::string GenerateRID(bool anisotropic = true, bool textureCoordWrap = false) noexcept;

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->PSSetSamplers(0U, 1U, state.GetAddressOf()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(anisotropic, textureCoordWrap); }
	};

	template<>
	struct is_resolvable_by_codex<Sampler>
	{
		static constexpr bool generate{ true };
	};

	inline std::shared_ptr<Sampler> Sampler::Get(Graphics& gfx, bool anisotropic, bool textureCoordWrap)
	{
		return Codex::Resolve<Sampler>(gfx, anisotropic, textureCoordWrap);
	}

	inline std::string Sampler::GenerateRID(bool anisotropic, bool textureCoordWrap) noexcept
	{
		return "#" + std::string(typeid(Sampler).name()) + "#" + std::to_string(anisotropic) + "#" + std::to_string(textureCoordWrap) + "#";
	}
}