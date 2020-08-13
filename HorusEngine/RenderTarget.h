#pragma once
#include "DepthStencil.h"

namespace GFX::Pipeline::Resource
{
	class RenderTarget : public IBufferResource
	{
		static const Microsoft::WRL::ComPtr<ID3D11RenderTargetView> nullTargetView;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> targetView = nullptr;
		D3D11_VIEWPORT viewport = { 0 };
		unsigned int width = 0;
		unsigned int height = 0;

		inline void BindViewport(Graphics& gfx) noexcept { GetContext(gfx)->RSSetViewports(1U, &viewport); }

	protected:
		constexpr RenderTarget() {}

		static Microsoft::WRL::ComPtr<ID3D11Texture2D> CreateTexture(Graphics& gfx, unsigned int width, unsigned int height, D3D11_TEXTURE2D_DESC& textureDesc);

		void InitializeTargetView(Graphics& gfx, D3D11_TEXTURE2D_DESC& textureDesc, Microsoft::WRL::ComPtr<ID3D11Texture2D> texture);

	public:
		RenderTarget(Graphics& gfx, unsigned int width, unsigned int height);
		virtual ~RenderTarget() = default;

		constexpr unsigned int GetWidth() const noexcept { return width; }
		constexpr unsigned int GetHeight() const noexcept { return height; }

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->OMSetRenderTargets(1U, targetView.GetAddressOf(), nullptr); BindViewport(gfx); }
		inline void Bind(Graphics& gfx, DepthStencil& depthStencil) noexcept { GetContext(gfx)->OMSetRenderTargets(1U, targetView.GetAddressOf(), depthStencil.depthStencilView.Get()); BindViewport(gfx); }

		virtual inline void Unbind(Graphics& gfx) noexcept { GetContext(gfx)->OMSetRenderTargets(1U, nullTargetView.GetAddressOf(), nullptr); }

		inline void Clear(Graphics& gfx) noexcept override { Clear(gfx, { 0.0f, 0.0f, 0.0f, 0.0f }); }
		inline void Clear(Graphics& gfx, const Data::ColorFloat4& color) noexcept override { GetContext(gfx)->ClearRenderTargetView(targetView.Get(), reinterpret_cast<const FLOAT*>(&color.col)); }
	};
}