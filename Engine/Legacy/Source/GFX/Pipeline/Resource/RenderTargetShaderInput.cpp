#include "GFX/Pipeline/Resource/RenderTargetShaderInput.h"
#include "GFX/Resource/GfxDebugName.h"
#include "GFX/Graphics.h"

namespace ZE::GFX::Pipeline::Resource
{
	RenderTargetShaderInput::RenderTargetShaderInput(Graphics& gfx, U32 slot, DXGI_FORMAT format, U32 slotUAV)
		: RenderTargetShaderInput(gfx, gfx.GetWidth(), gfx.GetHeight(), slot, format, slotUAV) {}

	RenderTargetShaderInput::RenderTargetShaderInput(Graphics& gfx, U32 width, U32 height, U32 slot, DXGI_FORMAT format, U32 slotUAV)
		: RenderTarget(width, height, format, slotUAV, gfx), slot(slot)
	{
		assert(slot < D3D11_PS_INPUT_REGISTER_COUNT);
		ZE_GFX_ENABLE_ALL(gfx);

		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture = CreateTexture(gfx, textureDesc);
		InitializeTargetView(gfx, textureDesc, texture);

		D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc = {};
		textureViewDesc.Format = textureDesc.Format;
		textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		textureViewDesc.Texture2D.MipLevels = 1;
		textureViewDesc.Texture2D.MostDetailedMip = 0;
		ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateShaderResourceView(texture.Get(), &textureViewDesc, &textureView));
		ZE_GFX_SET_ID(textureView.Get(), "RTI" + std::to_string(GetWidth()) + "x" +
			std::to_string(GetHeight()) + "#" + std::to_string(format) + "#" + std::to_string(slot));
	}

	void RenderTargetShaderInput::Unbind(Graphics& gfx) const
	{
		RenderTarget::Unbind(gfx);
		GetContext(gfx)->PSSetShaderResources(slot, 1, nullShaderResource.GetAddressOf());
		GetContext(gfx)->CSSetShaderResources(slot, 1, nullShaderResource.GetAddressOf());
	}
}