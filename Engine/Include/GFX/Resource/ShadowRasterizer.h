#pragma once
#include "GfxResPtr.h"

namespace ZE::GFX::Resource
{
	class ShadowRasterizer : public IBindable
	{
		S32 depthBias;
		float slopeBias;
		float biasClamp;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> state;

	public:
		ShadowRasterizer(Graphics& gfx, S32 depthBias, float slopeBias, float biasClamp);
		virtual ~ShadowRasterizer() = default;

		static GfxResPtr<ShadowRasterizer> Get(Graphics& gfx, S32 depthBias, float slopeBias, float biasClamp);

		constexpr S32 GetDepthBias() const noexcept { return depthBias; }
		constexpr float GetSlopeBias() const noexcept { return slopeBias; }
		constexpr float GetBiasClamp() const noexcept { return biasClamp; }

		void Bind(Graphics& gfx) const override { GetContext(gfx)->RSSetState(state.Get()); }
	};
}