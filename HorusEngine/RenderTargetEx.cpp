#include "RenderTargetEx.h"
#include "GfxExceptionMacros.h"

namespace GFX::Pipeline::Resource
{
	RenderTargetEx::RenderTargetEx(Graphics& gfx, unsigned int width, unsigned int height, UINT slot, std::vector<DXGI_FORMAT>&& formats)
		: IRenderTarget(gfx, width, height), slot(slot), count(static_cast<UINT>(formats.size()))
	{
		assert(count < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
		GFX_ENABLE_ALL(gfx);

		D3D11_TEXTURE2D_DESC textureDesc;
		textureDesc.Width = static_cast<UINT>(width);
		textureDesc.Height = static_cast<UINT>(height);
		textureDesc.ArraySize = 1U;
		textureDesc.MipLevels = 1U;
		textureDesc.SampleDesc.Count = 1U;
		textureDesc.SampleDesc.Quality = 0U;
		textureDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0U;
		textureDesc.MiscFlags = 0U;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture = nullptr;

		D3D11_RENDER_TARGET_VIEW_DESC targetViewDesc = {};
		targetViewDesc.ViewDimension = D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2D;
		targetViewDesc.Texture2D.MipSlice = 0U;

		D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc = {};
		textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D;
		textureViewDesc.Texture2D.MipLevels = 1U;
		textureViewDesc.Texture2D.MostDetailedMip = 0U;

		targetViews.resize(count);
		targetsArray = std::make_unique<ID3D11RenderTargetView* []>(count);
		textureViews.resize(count);
		texturesArray = std::make_unique<ID3D11ShaderResourceView* []>(count);
		nullTexturesArray = std::make_unique<ID3D11ShaderResourceView* []>(count);
		for (UINT i = 0U; i < count; ++i)
		{
			textureDesc.Format = formats.at(i);
			GFX_THROW_FAILED(GetDevice(gfx)->CreateTexture2D(&textureDesc, nullptr, &texture));

			targetViewDesc.Format = textureDesc.Format;
			GFX_THROW_FAILED(GetDevice(gfx)->CreateRenderTargetView(texture.Get(), &targetViewDesc, &targetViews.at(i)));
			targetsArray[i] = targetViews.at(i).Get();

			textureViewDesc.Format = textureDesc.Format;
			GFX_THROW_FAILED(GetDevice(gfx)->CreateShaderResourceView(texture.Get(), &textureViewDesc, &textureViews.at(i)));
			texturesArray[i] = textureViews.at(i).Get();
		}
	}

	void RenderTargetEx::Clear(Graphics& gfx, const Data::ColorFloat4& color) noexcept
	{
		for (auto& view : targetViews)
			GetContext(gfx)->ClearRenderTargetView(view.Get(), reinterpret_cast<const FLOAT*>(&color.col));
	}

	void RenderTargetEx::Clear(Graphics& gfx, const std::vector<Data::ColorFloat4>& colors) noexcept
	{
		size_t size = colors.size() >= targetViews.size() ? targetViews.size() : colors.size();
		size_t i = 0;
		for (; i < size; ++i)
			GetContext(gfx)->ClearRenderTargetView(targetViews.at(i).Get(), reinterpret_cast<const FLOAT*>(&colors.at(i).col));
		for (; i < targetViews.size(); ++i)
			GetContext(gfx)->ClearRenderTargetView(targetViews.at(i).Get(), reinterpret_cast<const FLOAT*>(&colors.back().col));
	}

	Surface RenderTargetEx::ToSurface(Graphics& gfx) const
	{
		GFX_ENABLE_ALL(gfx);

		// Create temporary texture to read from CPU side
		Microsoft::WRL::ComPtr<ID3D11Resource> resource;
		targetViews.at(0)->GetResource(&resource);
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
		for (unsigned int y = 0U; y < surface.GetHeight(); ++y)
		{
			const Surface::Pixel* row = reinterpret_cast<const Surface::Pixel*>(bytes + subResource.RowPitch * static_cast<size_t>(y));
			for (unsigned int x = 0U; x < GetWidth(); ++x)
				surface.PutPixel(x, y, *(row + x));
		}
		GFX_THROW_FAILED_INFO(GetContext(gfx)->Unmap(textureStaged.Get(), 0U));
		return surface;
	}
}