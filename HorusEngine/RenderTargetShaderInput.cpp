#include "RenderTargetShaderInput.h"
#include "GfxExceptionMacros.h"

namespace GFX::Pipeline::Resource
{
	const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> RenderTargetShaderInput::nullTextureView = nullptr;

	RenderTargetShaderInput::RenderTargetShaderInput(Graphics& gfx, unsigned int width, unsigned int height, UINT slot)
	{
		GFX_ENABLE_ALL(gfx);

		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture = CreateTexture(gfx, width, height, textureDesc);
		InitializeTargetView(gfx, textureDesc, texture);

		D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc = {};
		textureViewDesc.Format = textureDesc.Format;
		textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D;
		textureViewDesc.Texture2D.MipLevels = 1U;
		textureViewDesc.Texture2D.MostDetailedMip = 0U;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateShaderResourceView(texture.Get(), &textureViewDesc, &textureView));
	}
}