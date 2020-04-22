#pragma once
#include "Codex.h"

namespace GFX::Resource
{
	// How to perform lookups into texture
	class Sampler : public IBindable
	{
		Microsoft::WRL::ComPtr<ID3D11SamplerState> state;

	public:
		Sampler(Graphics& gfx);
		Sampler(const Sampler&) = delete;
		Sampler& operator=(const Sampler&) = delete;
		virtual ~Sampler() = default;

		static inline std::shared_ptr<Sampler> Get(Graphics& gfx) { return Codex::Resolve<Sampler>(gfx); }
		static inline std::string GenerateRID() noexcept { return "#" + std::string(typeid(Sampler).name()) + "#"; }

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->PSSetSamplers(0U, 1U, state.GetAddressOf()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(); }
	};

	template<>
	struct is_resolvable_by_codex<Sampler>
	{
		static constexpr bool generate{ true };
	};
}