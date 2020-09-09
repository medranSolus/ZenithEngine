#pragma once
#include "Codex.h"

namespace GFX::Resource
{
	class ShadowSampler : public IBindable
	{
		Microsoft::WRL::ComPtr<ID3D11SamplerState> state;

	public:
		ShadowSampler(Graphics& gfx);
		virtual ~ShadowSampler() = default;

		static inline std::shared_ptr<ShadowSampler> Get(Graphics& gfx);
		static inline std::string GenerateRID() noexcept;

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->PSSetSamplers(1U, 1U, state.GetAddressOf()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(); }
	};

	template<>
	struct is_resolvable_by_codex<ShadowSampler>
	{
		static constexpr bool generate{ true };
	};

	inline std::shared_ptr<ShadowSampler> ShadowSampler::Get(Graphics& gfx)
	{
		return Codex::Resolve<ShadowSampler>(gfx);
	}

	inline std::string ShadowSampler::GenerateRID() noexcept
	{
		return "#" + std::string(typeid(ShadowSampler).name()) + "#";
	}
}