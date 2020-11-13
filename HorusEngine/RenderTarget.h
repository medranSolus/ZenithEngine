#pragma once
#include "IRenderTarget.h"

namespace GFX::Pipeline::Resource
{
	class RenderTarget : public IRenderTarget
	{
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> targetView = nullptr;

	protected:
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

		inline RenderTarget(unsigned int width, unsigned int height, DXGI_FORMAT format, Graphics& gfx) noexcept : IRenderTarget(gfx, width, height), format(format) {}

		static Microsoft::WRL::ComPtr<ID3D11Texture2D> CreateTexture(Graphics& gfx, unsigned int width, unsigned int height, D3D11_TEXTURE2D_DESC& textureDesc, DXGI_FORMAT format);

		void InitializeTargetView(Graphics& gfx, D3D11_TEXTURE2D_DESC& textureDesc, Microsoft::WRL::ComPtr<ID3D11Texture2D> texture);

	public:
		RenderTarget(Graphics& gfx, DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);
		RenderTarget(Graphics& gfx, Microsoft::WRL::ComPtr<ID3D11Texture2D> texture, UINT size);
		RenderTarget(Graphics& gfx, unsigned int width, unsigned int height, DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);
		RenderTarget(Graphics& gfx, unsigned int width, unsigned int height, Microsoft::WRL::ComPtr<ID3D11Resource> backBuffer, DXGI_FORMAT format);
		virtual ~RenderTarget() = default;

		inline void BindTarget(Graphics& gfx) override { GetContext(gfx)->OMSetRenderTargets(1U, targetView.GetAddressOf(), nullptr); BindViewport(gfx); }
		inline void BindTarget(Graphics& gfx, DepthStencil& depthStencil) override { GetContext(gfx)->OMSetRenderTargets(1U, targetView.GetAddressOf(), depthStencil.depthStencilView.Get()); BindViewport(gfx); }

		inline void Clear(Graphics& gfx, const Data::ColorFloat4& color) noexcept override { GetContext(gfx)->ClearRenderTargetView(targetView.Get(), reinterpret_cast<const FLOAT*>(&color.col)); }
		Surface ToSurface(Graphics& gfx) const override;
	};
}