#pragma once
#include "IRenderTarget.h"

namespace GFX::Pipeline::Resource
{
	class RenderTarget : public IRenderTarget
	{
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> targetView = nullptr;

	protected:
		inline RenderTarget(unsigned int width, unsigned int height, Graphics& gfx) noexcept : IRenderTarget(gfx, width, height) {}

		static Microsoft::WRL::ComPtr<ID3D11Texture2D> CreateTexture(Graphics& gfx, unsigned int width, unsigned int height, D3D11_TEXTURE2D_DESC& textureDesc);

		void InitializeTargetView(Graphics& gfx, D3D11_TEXTURE2D_DESC& textureDesc, Microsoft::WRL::ComPtr<ID3D11Texture2D> texture);

	public:
		inline RenderTarget(Graphics& gfx) : RenderTarget(gfx, gfx.GetWidth(), gfx.GetHeight()) {}
		RenderTarget(Graphics& gfx, Microsoft::WRL::ComPtr<ID3D11Texture2D> texture, UINT size, UINT face);
		RenderTarget(Graphics& gfx, unsigned int width, unsigned int height);
		RenderTarget(Graphics& gfx, unsigned int width, unsigned int height, Microsoft::WRL::ComPtr<ID3D11Resource> backBuffer);
		virtual ~RenderTarget() = default;

		inline void BindTarget(Graphics& gfx) override { GetContext(gfx)->OMSetRenderTargets(1U, targetView.GetAddressOf(), nullptr); BindViewport(gfx); }
		inline void BindTarget(Graphics& gfx, DepthStencil& depthStencil) override { GetContext(gfx)->OMSetRenderTargets(1U, targetView.GetAddressOf(), depthStencil.depthStencilView.Get()); BindViewport(gfx); }

		inline void Clear(Graphics& gfx, const Data::ColorFloat4& color) noexcept override { GetContext(gfx)->ClearRenderTargetView(targetView.Get(), reinterpret_cast<const FLOAT*>(&color.col)); }

		Surface ToSurface(Graphics& gfx) const override;
	};
}