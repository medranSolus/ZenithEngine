#pragma once
#include "DepthStencil.h"

namespace GFX::Pipeline::Resource
{
	class DepthStencilShaderInput : public DepthStencil
	{
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView = nullptr;
		UINT slot;

		static inline DXGI_FORMAT UsageShaderInput(Usage usage) noexcept;

	public:
		DepthStencilShaderInput(Graphics& gfx, unsigned int width, unsigned int height, UINT slot, Usage usage = Usage::DepthStencil);

		constexpr UINT GetSlot() const noexcept { return slot; }
		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->PSSetShaderResources(slot, 1U, textureView.GetAddressOf()); }
	};
}