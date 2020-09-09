#pragma once
#include "DepthStencil.h"

namespace GFX::Pipeline::Resource
{
	class DepthStencilShaderInput : public DepthStencil
	{
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView = nullptr;
		UINT slot;

		static constexpr DXGI_FORMAT UsageShaderInput(Usage usage) noexcept;

	public:
		inline DepthStencilShaderInput(Graphics& gfx, UINT slot, Usage usage = Usage::DepthStencil)
			: DepthStencilShaderInput(gfx, gfx.GetWidth(), gfx.GetHeight(), slot, usage) {}
		DepthStencilShaderInput(Graphics& gfx, unsigned int width, unsigned int height, UINT slot, Usage usage = Usage::DepthStencil);
		virtual ~DepthStencilShaderInput() = default;

		constexpr UINT GetSlot() const noexcept { return slot; }

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->PSSetShaderResources(slot, 1U, textureView.GetAddressOf()); }
		inline void Unbind(Graphics& gfx) noexcept override { GetContext(gfx)->PSSetShaderResources(slot, 1U, nullShaderResource.GetAddressOf()); }
	};
}