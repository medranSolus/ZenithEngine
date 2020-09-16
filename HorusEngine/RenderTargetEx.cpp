#include "RenderTargetEx.h"
#include "GfxExceptionMacros.h"

namespace GFX::Pipeline::Resource
{
	RenderTargetEx::RenderTargetEx(Graphics& gfx, unsigned int width, unsigned int height, UINT count)
		: IRenderTarget(gfx, width, height), count(count)
	{
		assert(count < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
		GFX_ENABLE_ALL(gfx);

		D3D11_TEXTURE2D_DESC textureDesc;
		textureDesc.Width = static_cast<UINT>(width);
		textureDesc.Height = static_cast<UINT>(height);
		textureDesc.ArraySize = count;
		textureDesc.MipLevels = 1U;
		textureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1U;
		textureDesc.SampleDesc.Quality = 0U;
		textureDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags |= D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
		textureDesc.CPUAccessFlags = 0U;
		textureDesc.MiscFlags = 0U;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture = nullptr;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateTexture2D(&textureDesc, nullptr, &texture));

		D3D11_RENDER_TARGET_VIEW_DESC targetViewDesc = { 0 };
		targetViewDesc.Format = textureDesc.Format;
		targetViewDesc.ViewDimension = D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		targetViewDesc.Texture2DArray.ArraySize = count;
		targetViewDesc.Texture2DArray.FirstArraySlice = 0U;
		targetViewDesc.Texture2DArray.MipSlice = 0U;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateRenderTargetView(texture.Get(), &targetViewDesc, &targetViews));
	}

	void RenderTargetEx::Clear(Graphics& gfx, const Data::ColorFloat4& color) noexcept
	{
		GetContext(gfx)->ClearRenderTargetView(targetViews.Get(), reinterpret_cast<const FLOAT*>(&color.col));
	}

	Surface RenderTargetEx::ToSurface(Graphics& gfx) const
	{
		return Surface(100, 100);
	}
}