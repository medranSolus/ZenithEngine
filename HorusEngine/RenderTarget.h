#pragma once
#include "DepthStencil.h"

namespace GFX::Pipeline
{
	class RenderTarget : public GraphicsResource
	{
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> targetView = nullptr;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView = nullptr;

	public:
		RenderTarget(Graphics& gfx, unsigned int width, unsigned int height);
		RenderTarget(const RenderTarget&) = delete;
		RenderTarget& operator=(const RenderTarget&) = delete;
		virtual ~RenderTarget() = default;

		inline void BindTexture(Graphics& gfx, UINT slot) noexcept { GetContext(gfx)->PSSetShaderResources(slot, 1U, textureView.GetAddressOf()); }
		inline void BindTarget(Graphics& gfx) noexcept { GetContext(gfx)->OMSetRenderTargets(1U, targetView.GetAddressOf(), nullptr); }
		inline void BindTarget(Graphics& gfx, DepthStencil& depthStencil) noexcept { GetContext(gfx)->OMSetRenderTargets(1U, targetView.GetAddressOf(), depthStencil.depthStencilView.Get()); }
	};
}