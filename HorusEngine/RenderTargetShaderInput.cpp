#include "RenderTargetShaderInput.h"
#include "GfxExceptionMacros.h"

namespace GFX::Pipeline::Resource
{
	const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> RenderTargetShaderInput::nullTextureView = nullptr;

	RenderTargetShaderInput::RenderTargetShaderInput(Graphics& gfx, unsigned int width, unsigned int height, UINT slot) : RenderTarget(width, height), slot(slot)
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

	Surface RenderTargetShaderInput::ToSurface(Graphics& gfx) const
	{
		GFX_ENABLE_ALL(gfx);

		// Create temporary texture to read from CPU side
		Microsoft::WRL::ComPtr<ID3D11Resource> resource;
		textureView->GetResource(&resource);
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		resource.As(&texture);
		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		texture->GetDesc(&textureDesc);
		textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_READ;
		textureDesc.Usage = D3D11_USAGE::D3D11_USAGE_STAGING;
		textureDesc.BindFlags = 0U;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> textureStaged;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateTexture2D(&textureDesc, nullptr, &textureStaged));
		GFX_THROW_FAILED_INFO(GetContext(gfx)->CopyResource(textureStaged.Get(), texture.Get()));

		Surface surface(GetWidth(), GetHeight());
		D3D11_MAPPED_SUBRESOURCE subResource = { 0 };
		GFX_THROW_FAILED_INFO(GetContext(gfx)->Map(textureStaged.Get(), 0U, D3D11_MAP::D3D11_MAP_READ, 0U, &subResource));
		const char* bytes = static_cast<const char*>(subResource.pData);
		for (unsigned int y = 0U; y < GetHeight(); ++y)
		{
			const Surface::Pixel* row = reinterpret_cast<const Surface::Pixel*>(bytes + subResource.RowPitch * static_cast<size_t>(y));
			for (unsigned int x = 0U; x < GetWidth(); ++x)
				surface.PutPixel(x, y, *(row + x));
		}
		GFX_THROW_FAILED_INFO(GetContext(gfx)->Unmap(textureStaged.Get(), 0U));
		return surface;
	}
}