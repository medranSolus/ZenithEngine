#pragma once
#include "RenderTarget.h"

namespace GFX::Pipeline::Resource
{
	class RenderTargetShaderInput : public RenderTarget
	{
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView = nullptr;
		UINT slot;

	public:
		inline RenderTargetShaderInput(Graphics& gfx, UINT slot, DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM)
			: RenderTargetShaderInput(gfx, gfx.GetWidth(), gfx.GetHeight(), slot, format) {}
		RenderTargetShaderInput(Graphics& gfx, unsigned int width, unsigned int height, UINT slot, DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);
		virtual ~RenderTargetShaderInput() = default;

		constexpr UINT GetSlot() const noexcept { return slot; }

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->PSSetShaderResources(slot, 1U, textureView.GetAddressOf()); }
		inline void Unbind(Graphics& gfx) noexcept override { GetContext(gfx)->PSSetShaderResources(slot, 1U, nullShaderResource.GetAddressOf()); }
	};
}