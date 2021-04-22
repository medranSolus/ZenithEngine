#pragma once
#include "IRenderTarget.h"

namespace GFX::Pipeline::Resource
{
	class RenderTarget : public IRenderTarget
	{
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> targetView = nullptr;

	protected:
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

		RenderTarget(U32 width, U32 height, DXGI_FORMAT format, Graphics& gfx) noexcept
			: IRenderTarget(gfx, width, height), format(format) {}

		static Microsoft::WRL::ComPtr<ID3D11Texture2D> CreateTexture(Graphics& gfx, U32 width, U32 height, D3D11_TEXTURE2D_DESC& textureDesc, DXGI_FORMAT format);

		void InitializeTargetView(Graphics& gfx, D3D11_TEXTURE2D_DESC& textureDesc, Microsoft::WRL::ComPtr<ID3D11Texture2D> texture);

	public:
		RenderTarget(Graphics& gfx, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
		RenderTarget(Graphics& gfx, Microsoft::WRL::ComPtr<ID3D11Texture2D> texture, U32 size);
		RenderTarget(Graphics& gfx, U32 width, U32 height, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
		RenderTarget(Graphics& gfx, U32 width, U32 height, Microsoft::WRL::ComPtr<ID3D11Resource> backBuffer, DXGI_FORMAT format);
		virtual ~RenderTarget() = default;

		void BindTarget(Graphics& gfx) const override { GetContext(gfx)->OMSetRenderTargets(1, targetView.GetAddressOf(), nullptr); BindViewport(gfx); }

#ifdef _MODE_DEBUG
		std::string GetRID() const noexcept override;
#endif
		void BindTarget(Graphics& gfx, DepthStencil& depthStencil) const override;
		void Clear(Graphics& gfx, const ColorF4& color) override;
		Surface ToSurface(Graphics& gfx) const override;
	};
}