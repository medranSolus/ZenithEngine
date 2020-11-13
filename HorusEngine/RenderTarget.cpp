#include "RenderTarget.h"
#include "Graphics.h"
#include "GfxExceptionMacros.h"

namespace GFX::Pipeline::Resource
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> RenderTarget::CreateTexture(Graphics& gfx, unsigned int width, unsigned int height,
		D3D11_TEXTURE2D_DESC& textureDesc, DXGI_FORMAT format)
	{
		GFX_ENABLE_ALL(gfx);

		textureDesc.Width = static_cast<UINT>(width);
		textureDesc.Height = static_cast<UINT>(height);
		textureDesc.ArraySize = 1U;
		textureDesc.MipLevels = 1U;
		textureDesc.Format = format;
		textureDesc.SampleDesc.Count = 1U;
		textureDesc.SampleDesc.Quality = 0U;
		textureDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags |= D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
		textureDesc.CPUAccessFlags = 0U;
		textureDesc.MiscFlags = 0U;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture = nullptr;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateTexture2D(&textureDesc, nullptr, &texture));

		return texture;
	}

	void RenderTarget::InitializeTargetView(Graphics& gfx, D3D11_TEXTURE2D_DESC& textureDesc, Microsoft::WRL::ComPtr<ID3D11Texture2D> texture)
	{
		GFX_ENABLE_ALL(gfx);

		D3D11_RENDER_TARGET_VIEW_DESC targetViewDesc = {};
		targetViewDesc.Format = textureDesc.Format;
		targetViewDesc.ViewDimension = D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2D;
		targetViewDesc.Texture2D = D3D11_TEX2D_RTV{ 0 };
		GFX_THROW_FAILED(GetDevice(gfx)->CreateRenderTargetView(texture.Get(), &targetViewDesc, &targetView));
		SET_DEBUG_NAME(targetView.Get(), "RT" + std::to_string(GetWidth()) + "x" + std::to_string(GetHeight()) + "#" + std::to_string(format));
	}

	RenderTarget::RenderTarget(Graphics& gfx, DXGI_FORMAT format)
		: RenderTarget(gfx, gfx.GetWidth(), gfx.GetHeight(), format) {}

	RenderTarget::RenderTarget(Graphics& gfx, Microsoft::WRL::ComPtr<ID3D11Texture2D> texture, UINT size)
		: IRenderTarget(gfx, size, size), format(DXGI_FORMAT_R32_FLOAT)
	{
		GFX_ENABLE_ALL(gfx);

		D3D11_RENDER_TARGET_VIEW_DESC targetViewDesc = {};
		targetViewDesc.Format = format;
		targetViewDesc.ViewDimension = D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		targetViewDesc.Texture2DArray.MipSlice = 0U;
		targetViewDesc.Texture2DArray.ArraySize = 6U;
		targetViewDesc.Texture2DArray.FirstArraySlice = 0U;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateRenderTargetView(texture.Get(), &targetViewDesc, &targetView));
		SET_DEBUG_NAME(targetView.Get(), "RT" + std::to_string(GetWidth()) + "x" + std::to_string(GetHeight()) + "#" + std::to_string(format));
	}

	RenderTarget::RenderTarget(Graphics& gfx, unsigned int width, unsigned int height, DXGI_FORMAT format)
		: IRenderTarget(gfx, width, height), format(format)
	{
		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		textureDesc.BindFlags = 0U;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture = CreateTexture(gfx, width, height, textureDesc, format);
		InitializeTargetView(gfx, textureDesc, texture);
	}

	RenderTarget::RenderTarget(Graphics& gfx, unsigned int width, unsigned int height, Microsoft::WRL::ComPtr<ID3D11Resource> backBuffer, DXGI_FORMAT format)
		: IRenderTarget(gfx, width, height), format(format)
	{
		GFX_ENABLE_ALL(gfx);
		GFX_THROW_FAILED(GetDevice(gfx)->CreateRenderTargetView(backBuffer.Get(), nullptr, &targetView));
	}

	Surface RenderTarget::ToSurface(Graphics& gfx) const
	{
		GFX_ENABLE_ALL(gfx);

		// Create temporary texture to read from CPU side
		Microsoft::WRL::ComPtr<ID3D11Resource> resource;
		targetView->GetResource(&resource);
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		resource->QueryInterface(IID_PPV_ARGS(&texture));
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