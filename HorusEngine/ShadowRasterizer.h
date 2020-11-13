#pragma once
#include "GfxResPtr.h"

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
		virtual ~ShadowRasterizer() = default;

		static inline GfxResPtr<ShadowRasterizer> Get(Graphics& gfx, int depthBias, float slopeBias, float biasClamp);

		constexpr int GetDepthBias() const noexcept { return depthBias; }
		constexpr float GetSlopeBias() const noexcept { return slopeBias; }
		constexpr float GetBiasClamp() const noexcept { return biasClamp; }

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->RSSetState(state.Get()); }
		inline std::string GetRID() const noexcept override { return IBindable::GetNoCodexRID(); }
	};

	inline GfxResPtr<ShadowRasterizer> ShadowRasterizer::Get(Graphics& gfx, int depthBias, float slopeBias, float biasClamp)
	{
		return GfxResPtr<ShadowRasterizer>(gfx, depthBias, slopeBias, biasClamp);
	}
}