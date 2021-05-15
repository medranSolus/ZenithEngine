#include "GFX/Pipeline/Resource/RenderTargetEx.h"
#include "GFX/Resource/GfxDebugName.h"
#include "GFX/Graphics.h"

namespace ZE::GFX::Pipeline::Resource
{
	RenderTargetEx::RenderTargetEx(Graphics& gfx, U32 width, U32 height, U32 slot, std::vector<DXGI_FORMAT>&& formats)
		: IRenderTarget(gfx, width, height), slot(slot), count(static_cast<U32>(formats.size()))
	{
		assert(count < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
		ZE_GFX_ENABLE_ALL(gfx);

		D3D11_TEXTURE2D_DESC textureDesc;
		textureDesc.Width = static_cast<UINT>(width);
		textureDesc.Height = static_cast<UINT>(height);
		textureDesc.ArraySize = 1;
		textureDesc.MipLevels = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture = nullptr;

		D3D11_RENDER_TARGET_VIEW_DESC targetViewDesc = {};
		targetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		targetViewDesc.Texture2D.MipSlice = 0;

		D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc = {};
		textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		textureViewDesc.Texture2D.MipLevels = 1;
		textureViewDesc.Texture2D.MostDetailedMip = 0;

		targetViews.resize(count);
		targetsArray = std::make_unique<ID3D11RenderTargetView* []>(count);
		textureViews.resize(count);
		texturesArray = std::make_unique<ID3D11ShaderResourceView* []>(count);
		nullTexturesArray = std::make_unique<ID3D11ShaderResourceView* []>(count);
		for (U32 i = 0; i < count; ++i)
		{
			textureDesc.Format = formats.at(i);
			ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateTexture2D(&textureDesc, nullptr, &texture));

			targetViewDesc.Format = textureDesc.Format;
			ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateRenderTargetView(texture.Get(), &targetViewDesc, &targetViews.at(i)));
			targetsArray[i] = targetViews.at(i).Get();
			ZE_GFX_SET_ID(targetViews.at(i).Get(), "RTE" + std::to_string(GetWidth()) + "x" + std::to_string(GetHeight()) + "#" +
				std::to_string(textureDesc.Format) + "#" + std::to_string(slot) + "#" + std::to_string(count));

			textureViewDesc.Format = textureDesc.Format;
			ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateShaderResourceView(texture.Get(), &textureViewDesc, &textureViews.at(i)));
			texturesArray[i] = textureViews.at(i).Get();
			ZE_GFX_SET_ID_EX(textureViews.at(i).Get());
		}
	}

	GfxResPtr<RenderTargetEx> RenderTargetEx::Get(Graphics& gfx, U32 slot, std::vector<DXGI_FORMAT>&& formats)
	{
		return GfxResPtr<Resource::RenderTargetEx>(gfx, gfx.GetWidth(), gfx.GetHeight(), slot, std::forward<std::vector<DXGI_FORMAT>>(formats));
	}

	void RenderTargetEx::BindTarget(Graphics& gfx) const
	{
		GetContext(gfx)->OMSetRenderTargets(count, targetsArray.get(), nullptr);
		BindViewport(gfx);
	}

	void RenderTargetEx::BindTarget(Graphics& gfx, DepthStencil& depthStencil) const
	{
		GetContext(gfx)->OMSetRenderTargets(count, targetsArray.get(), depthStencil.depthStencilView.Get());
		BindViewport(gfx);
	}

	void RenderTargetEx::Clear(Graphics& gfx, const ColorF4& color)
	{
		for (auto& view : targetViews)
			GetContext(gfx)->ClearRenderTargetView(view.Get(), reinterpret_cast<const FLOAT*>(&color.RGBA));
	}

	void RenderTargetEx::Clear(Graphics& gfx, const std::vector<ColorF4>& colors)
	{
		U64 size = colors.size() >= targetViews.size() ? targetViews.size() : colors.size();
		U64 i = 0;
		for (; i < size; ++i)
			GetContext(gfx)->ClearRenderTargetView(targetViews.at(i).Get(), reinterpret_cast<const FLOAT*>(&colors.at(i).RGBA));
		for (; i < targetViews.size(); ++i)
			GetContext(gfx)->ClearRenderTargetView(targetViews.at(i).Get(), reinterpret_cast<const FLOAT*>(&colors.back().RGBA));
	}

	Surface RenderTargetEx::ToSurface(Graphics& gfx) const
	{
		ZE_GFX_ENABLE_ALL(gfx);

		// Create temporary texture to read from CPU side
		Microsoft::WRL::ComPtr<ID3D11Resource> resource;
		targetViews.at(0)->GetResource(&resource);
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		resource->QueryInterface(IID_PPV_ARGS(&texture));

		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		texture->GetDesc(&textureDesc);
		textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		textureDesc.Usage = D3D11_USAGE_STAGING;
		textureDesc.BindFlags = 0;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> textureStaged;
		ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateTexture2D(&textureDesc, nullptr, &textureStaged));
		ZE_GFX_THROW_FAILED_INFO(GetContext(gfx)->CopyResource(textureStaged.Get(), texture.Get()));

		Surface surface(GetWidth(), GetHeight());
		D3D11_MAPPED_SUBRESOURCE subResource = { 0 };
		ZE_GFX_THROW_FAILED_INFO(GetContext(gfx)->Map(textureStaged.Get(), 0, D3D11_MAP_READ, 0, &subResource));
		const U8* bytes = static_cast<const U8*>(subResource.pData);
		for (U32 y = 0; y < surface.GetHeight(); ++y)
		{
			const Pixel* row = reinterpret_cast<const Pixel*>(bytes + subResource.RowPitch * static_cast<U64>(y));
			for (U32 x = 0; x < GetWidth(); ++x)
				surface.PutPixel(x, y, *(row + x));
		}
		ZE_GFX_THROW_FAILED_INFO(GetContext(gfx)->Unmap(textureStaged.Get(), 0));
		return surface;
	}
}