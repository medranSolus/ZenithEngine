#pragma once
#include "DepthStencil.h"

namespace GFX::Pipeline::Resource
{
	class RenderTarget : public IBufferResource
	{
		static const Microsoft::WRL::ComPtr<ID3D11RenderTargetView> nullTargetView;
		static const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> nullTextureView;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> targetView = nullptr;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView = nullptr;
		D3D11_VIEWPORT viewport = { 0 };

		inline void BindViewport(Graphics& gfx) noexcept { GetContext(gfx)->RSSetViewports(1U, &viewport); }

	public:
		RenderTarget(Graphics& gfx, unsigned int width, unsigned int height);
		virtual ~RenderTarget() = default;

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->OMSetRenderTargets(1U, targetView.GetAddressOf(), nullptr); BindViewport(gfx); }
		inline void Bind(Graphics& gfx, DepthStencil& depthStencil) noexcept { GetContext(gfx)->OMSetRenderTargets(1U, targetView.GetAddressOf(), depthStencil.depthStencilView.Get()); BindViewport(gfx); }
		inline void BindTexture(Graphics& gfx, UINT slot) noexcept { GetContext(gfx)->PSSetShaderResources(slot, 1U, textureView.GetAddressOf()); }

		inline void Unbind(Graphics& gfx) noexcept { GetContext(gfx)->OMSetRenderTargets(1U, nullTargetView.GetAddressOf(), nullptr); }
		inline void UnbindTexture(Graphics& gfx, UINT slot) noexcept { GetContext(gfx)->PSSetShaderResources(slot, 1U, nullTextureView.GetAddressOf()); }

		inline void Clear(Graphics& gfx) noexcept override { Clear(gfx, { 0.0f, 0.0f, 0.0f, 0.0f }); }
		inline void Clear(Graphics& gfx, const Data::ColorFloat4& color) noexcept override { GetContext(gfx)->ClearRenderTargetView(targetView.Get(), reinterpret_cast<const FLOAT*>(&color.col)); }
	};
}