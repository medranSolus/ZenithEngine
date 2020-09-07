#pragma once
#include "IBindable.h"

namespace GFX::Resource
{
	class ShadowRasterizer : public IBindable
	{
		int depthBias;
		float slopeBias;
		float biasClamp;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> state;

	public:
		ShadowRasterizer(Graphics& gfx, int depthBias, float slopeBias, float biasClamp);
		ShadowRasterizer(const ShadowRasterizer&) = delete;
		ShadowRasterizer& operator=(const ShadowRasterizer&) = delete;
		virtual ~ShadowRasterizer() = default;

		static inline std::shared_ptr<ShadowRasterizer> Get(Graphics& gfx, int depthBias, float slopeBias, float biasClamp) { return std::make_shared<ShadowRasterizer>(gfx, depthBias, slopeBias, biasClamp); }

		constexpr int GetDepthBias() const noexcept { return depthBias; }
		constexpr float GetSlopeBias() const noexcept { return slopeBias; }
		constexpr float GetBiasClamp() const noexcept { return biasClamp; }

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->RSSetState(state.Get()); }
		inline std::string GetRID() const noexcept override { return "?"; }
	};
}