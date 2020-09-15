#pragma once
#include "IRenderTarget.h"

namespace GFX::Pipeline::Resource
{
	class RenderTargetEx : public IRenderTarget
	{
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> targetViews = nullptr;
		UINT count;

	public:
		RenderTargetEx(Graphics& gfx, unsigned int width, unsigned int height, UINT count);
		virtual ~RenderTargetEx() = default;

		inline void BindTarget(Graphics& gfx) override { GetContext(gfx)->OMSetRenderTargets(count, targetViews.GetAddressOf(), nullptr); BindViewport(gfx); }
		inline void BindTarget(Graphics& gfx, DepthStencil& depthStencil) override { GetContext(gfx)->OMSetRenderTargets(count, targetViews.GetAddressOf(), depthStencil.depthStencilView.Get()); BindViewport(gfx); }

		void Clear(Graphics& gfx, const Data::ColorFloat4& color) noexcept override;
		Surface ToSurface(Graphics& gfx) const override;
	};
}