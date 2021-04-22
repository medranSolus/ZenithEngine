#include "GFX/Pipeline/Resource/RenderTargetShaderInput.h"
#include "GFX/Resource/GfxDebugName.h"
#include "GFX/Graphics.h"

namespace GFX::Pipeline::Resource
{
	RenderTargetShaderInput::RenderTargetShaderInput(Graphics& gfx, U32 slot, DXGI_FORMAT format)
		: RenderTargetShaderInput(gfx, gfx.GetWidth(), gfx.GetHeight(), slot, format) {}

	RenderTargetShaderInput::RenderTargetShaderInput(Graphics& gfx, U32 width, U32 height, U32 slot, DXGI_FORMAT format)
		: RenderTarget(width, height, format, gfx), slot(slot)
	{
		GFX_ENABLE_ALL(gfx);

		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture = CreateTexture(gfx, width, height, textureDesc, format);
		InitializeTargetView(gfx, textureDesc, texture);

		D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc = {};
		textureViewDesc.Format = textureDesc.Format;
		textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		textureViewDesc.Texture2D.MipLevels = 1;
		textureViewDesc.Texture2D.MostDetailedMip = 0;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateShaderResourceView(texture.Get(), &textureViewDesc, &textureView));
		GFX_SET_ID(textureView.Get(), "RTI" + std::to_string(GetWidth()) + "x" +
			std::to_string(GetHeight()) + "#" + std::to_string(format) + "#" + std::to_string(slot));
	}
}