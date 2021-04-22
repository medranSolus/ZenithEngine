#pragma once
#include "DepthStencil.h"

namespace GFX::Pipeline::Resource
{
	class DepthStencilShaderInput : public DepthStencil
	{
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView = nullptr;
		U32 slot;

		static constexpr DXGI_FORMAT UsageShaderInput(Usage usage) noexcept;

	public:
		DepthStencilShaderInput(Graphics& gfx, U32 slot, Usage usage = Usage::DepthStencil);
		DepthStencilShaderInput(Graphics& gfx, U32 width, U32 height, U32 slot, Usage usage = Usage::DepthStencil);
		virtual ~DepthStencilShaderInput() = default;

		constexpr U32 GetSlot() const noexcept { return slot; }

		void Bind(Graphics& gfx) const override { GetContext(gfx)->PSSetShaderResources(slot, 1, textureView.GetAddressOf()); }
		void Unbind(Graphics& gfx) const override { GetContext(gfx)->PSSetShaderResources(slot, 1, nullShaderResource.GetAddressOf()); }
	};
}