#include "Texture.h"
#include "Surface.h"
#include "Codex.h"
#include "GfxExceptionMacros.h"

namespace GFX::Resource
{
	Texture::Texture(Graphics& gfx, const std::string& path, UINT slot) : slot(slot), path(path)
	{
		GFX_ENABLE_ALL(gfx);
		Surface surface(path);
		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		textureDesc.Width = surface.GetWidth();
		textureDesc.Height = surface.GetHeight();
		textureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM; // Same as backbuffer
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA resData = { 0 };
		resData.pSysMem = surface.GetBuffer();
		resData.SysMemPitch = surface.GetWidth() * sizeof(Surface::Pixel);
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateTexture2D(&textureDesc, &resData, &texture));

		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.Format = textureDesc.Format;
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D10_1_SRV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipLevels = 1;
		viewDesc.Texture2D.MostDetailedMip = 0;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateShaderResourceView(texture.Get(), &viewDesc, &textureView));
	}
}