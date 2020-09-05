#pragma once
#include "DepthStencil.h"

namespace GFX::Pipeline::Resource
{
	class RenderTarget : public IBufferResource
	{
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> targetView = nullptr;

	protected:
		inline RenderTarget(unsigned int width, unsigned int height) noexcept : IBufferResource(width, height) {}

		static Microsoft::WRL::ComPtr<ID3D11Texture2D> CreateTexture(Graphics& gfx, unsigned int width, unsigned int height, D3D11_TEXTURE2D_DESC& textureDesc);

		void InitializeTargetView(Graphics& gfx, D3D11_TEXTURE2D_DESC& textureDesc, Microsoft::WRL::ComPtr<ID3D11Texture2D> texture);

	public:
		RenderTarget(Graphics& gfx, unsigned int width, unsigned int height);
		RenderTarget(Graphics& gfx, unsigned int width, unsigned int height, Microsoft::WRL::ComPtr<ID3D11Resource> backBuffer);
		virtual ~RenderTarget() = default;

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->OMSetRenderTargets(1U, targetView.GetAddressOf(), nullptr); BindViewport(gfx); }
		inline void Bind(Graphics& gfx, DepthStencil& depthStencil) noexcept { GetContext(gfx)->OMSetRenderTargets(1U, targetView.GetAddressOf(), depthStencil.depthStencilView.Get()); BindViewport(gfx); }

		inline void Clear(Graphics& gfx) noexcept override { Clear(gfx, { 0.0f, 0.0f, 0.0f, 0.0f }); }
		inline void Clear(Graphics& gfx, const Data::ColorFloat4& color) noexcept override { GetContext(gfx)->ClearRenderTargetView(targetView.Get(), reinterpret_cast<const FLOAT*>(&color.col)); }

		Surface ToSurface(Graphics& gfx) const;
	};
}