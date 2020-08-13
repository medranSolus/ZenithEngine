#include "DepthStencil.h"
#include "GfxExceptionMacros.h"

namespace GFX::Pipeline::Resource
{
	DepthStencil::DepthStencil(Graphics& gfx, unsigned int width, unsigned int height)
	{
		GFX_ENABLE_ALL(gfx);

		D3D11_TEXTURE2D_DESC depthTexDesc = { 0 };
		depthTexDesc.Width = static_cast<UINT>(width);
		depthTexDesc.Height = static_cast<UINT>(height);
		depthTexDesc.MipLevels = 0U;
		depthTexDesc.ArraySize = 1U;
		depthTexDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthTexDesc.SampleDesc.Count = 1U; // Antialiasing stuff
		depthTexDesc.SampleDesc.Quality = 0U;
		depthTexDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		depthTexDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthTexture = nullptr;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateTexture2D(&depthTexDesc, nullptr, &depthTexture));

		D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc = {};
		depthViewDesc.Format = depthTexDesc.Format;
		depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
		depthViewDesc.Texture2D = D3D11_TEX2D_DSV{ 0 };
		GFX_THROW_FAILED(GetDevice(gfx)->CreateDepthStencilView(depthTexture.Get(), &depthViewDesc, &depthStencilView));
	}
}