#pragma once
#include "RenderTarget.h"

namespace GFX::Pipeline::Resource
{
	class RenderTargetShaderInput : public RenderTarget
	{
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView = nullptr;
		UINT slot;

	public:
		RenderTargetShaderInput(Graphics& gfx, unsigned int width, unsigned int height, UINT slot);
		virtual ~RenderTargetShaderInput() = default;

		constexpr UINT GetSlot() const noexcept { return slot; }

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->PSSetShaderResources(slot, 1U, textureView.GetAddressOf()); }
		inline void Unbind(Graphics& gfx) noexcept override { GetContext(gfx)->PSSetShaderResources(slot, 1U, nullShaderResource.GetAddressOf()); }
	};
}