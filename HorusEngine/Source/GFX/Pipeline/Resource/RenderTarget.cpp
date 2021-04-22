#include "GFX/Pipeline/Resource/RenderTarget.h"
#include "GFX/Resource/GfxDebugName.h"
#include "GFX/Graphics.h"

namespace GFX::Pipeline::Resource
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> RenderTarget::CreateTexture(Graphics& gfx, U32 width, U32 height,
		D3D11_TEXTURE2D_DESC& textureDesc, DXGI_FORMAT format)
	{
		GFX_ENABLE_ALL(gfx);

		textureDesc.Width = static_cast<UINT>(width);
		textureDesc.Height = static_cast<UINT>(height);
		textureDesc.ArraySize = 1;
		textureDesc.MipLevels = 1U;
		textureDesc.Format = format;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture = nullptr;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateTexture2D(&textureDesc, nullptr, &texture));

		return texture;
	}

	void RenderTarget::InitializeTargetView(Graphics& gfx,
		D3D11_TEXTURE2D_DESC& textureDesc, Microsoft::WRL::ComPtr<ID3D11Texture2D> texture)
	{
		GFX_ENABLE_ALL(gfx);

		D3D11_RENDER_TARGET_VIEW_DESC targetViewDesc = {};
		targetViewDesc.Format = textureDesc.Format;
		targetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		targetViewDesc.Texture2D = D3D11_TEX2D_RTV{ 0 };
		GFX_THROW_FAILED(GetDevice(gfx)->CreateRenderTargetView(texture.Get(), &targetViewDesc, &targetView));
		GFX_SET_RID(targetView.Get());
	}

	RenderTarget::RenderTarget(Graphics& gfx, DXGI_FORMAT format)
		: RenderTarget(gfx, gfx.GetWidth(), gfx.GetHeight(), format) {}

	RenderTarget::RenderTarget(Graphics& gfx, Microsoft::WRL::ComPtr<ID3D11Texture2D> texture, U32 size)
		: IRenderTarget(gfx, size, size), format(DXGI_FORMAT_R32_FLOAT)
	{
		GFX_ENABLE_ALL(gfx);

		D3D11_RENDER_TARGET_VIEW_DESC targetViewDesc = {};
		targetViewDesc.Format = format;
		targetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		targetViewDesc.Texture2DArray.MipSlice = 0;
		targetViewDesc.Texture2DArray.ArraySize = 6;
		targetViewDesc.Texture2DArray.FirstArraySlice = 0;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateRenderTargetView(texture.Get(), &targetViewDesc, &targetView));
		GFX_SET_RID(targetView.Get());
	}

	RenderTarget::RenderTarget(Graphics& gfx, U32 width, U32 height, DXGI_FORMAT format)
		: IRenderTarget(gfx, width, height), format(format)
	{
		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		textureDesc.BindFlags = 0;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture = CreateTexture(gfx, width, height, textureDesc, format);
		InitializeTargetView(gfx, textureDesc, texture);
	}

	RenderTarget::RenderTarget(Graphics& gfx, U32 width, U32 height,
		Microsoft::WRL::ComPtr<ID3D11Resource> backBuffer, DXGI_FORMAT format)
		: IRenderTarget(gfx, width, height), format(format)
	{
		GFX_ENABLE_ALL(gfx);
		GFX_THROW_FAILED(GetDevice(gfx)->CreateRenderTargetView(backBuffer.Get(), nullptr, &targetView));
	}

#ifdef _MODE_DEBUG
	std::string RenderTarget::GetRID() const noexcept
	{
		return "RT" + std::to_string(GetWidth()) + "x" + std::to_string(GetHeight()) + "#" + std::to_string(format);
	}
#endif

	void RenderTarget::BindTarget(Graphics& gfx, DepthStencil& depthStencil) const
	{
		GetContext(gfx)->OMSetRenderTargets(1, targetView.GetAddressOf(), depthStencil.depthStencilView.Get());
		BindViewport(gfx);
	}

	void RenderTarget::Clear(Graphics& gfx, const ColorF4& color)
	{
		GetContext(gfx)->ClearRenderTargetView(targetView.Get(), reinterpret_cast<const FLOAT*>(&color.RGBA));
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
		textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		textureDesc.Usage = D3D11_USAGE_STAGING;
		textureDesc.BindFlags = 0;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> textureStaged;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateTexture2D(&textureDesc, nullptr, &textureStaged));
		GFX_THROW_FAILED_INFO(GetContext(gfx)->CopyResource(textureStaged.Get(), texture.Get()));

		Surface surface(GetWidth(), GetHeight());
		D3D11_MAPPED_SUBRESOURCE subResource = { 0 };
		GFX_THROW_FAILED_INFO(GetContext(gfx)->Map(textureStaged.Get(), 0, D3D11_MAP_READ, 0, &subResource));
		const U8* bytes = static_cast<const U8*>(subResource.pData);
		for (U32 y = 0; y < GetHeight(); ++y)
		{
			const Pixel* row = reinterpret_cast<const Pixel*>(bytes + subResource.RowPitch * static_cast<U64>(y));
			for (U32 x = 0; x < GetWidth(); ++x)
				surface.PutPixel(x, y, *(row + x));
		}
		GFX_THROW_FAILED_INFO(GetContext(gfx)->Unmap(textureStaged.Get(), 0));
		return surface;
	}
}