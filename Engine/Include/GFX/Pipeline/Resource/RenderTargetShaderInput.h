#pragma once
#include "RenderTarget.h"

namespace ZE::GFX::Pipeline::Resource
{
	class RenderTargetShaderInput : public RenderTarget
	{
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView = nullptr;
		U32 slot;

	public:
		RenderTargetShaderInput(Graphics& gfx, U32 slot, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
		RenderTargetShaderInput(Graphics& gfx, U32 width, U32 height, U32 slot, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
		virtual ~RenderTargetShaderInput() = default;

		constexpr U32 GetSlot() const noexcept { return slot; }

		void Bind(Graphics& gfx) const override { GetContext(gfx)->PSSetShaderResources(slot, 1, textureView.GetAddressOf()); }
		void Unbind(Graphics& gfx) const override { GetContext(gfx)->PSSetShaderResources(slot, 1, nullShaderResource.GetAddressOf()); }
	};
}