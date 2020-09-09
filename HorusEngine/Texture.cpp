#include "Texture.h"
#include "Surface.h"
#include "GfxExceptionMacros.h"
#include <thread>

namespace GFX::Resource
{
	Texture::Texture(Graphics& gfx, const std::string& path, UINT slot, bool alphaEnable) : slot(slot), path(path)
	{
		GFX_ENABLE_ALL(gfx);
		Surface surface(path);
		std::unique_ptr<std::thread> checkAlpha = nullptr;
		if (alphaEnable)
			checkAlpha = std::make_unique<std::thread>([&surface, this]() { alpha = surface.HasAlpha(); });

		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		textureDesc.Width = static_cast<UINT>(surface.GetWidth());
		textureDesc.Height = static_cast<UINT>(surface.GetHeight());
		textureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM; // Same as backbuffer
		textureDesc.MipLevels = 0;
		textureDesc.ArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_FLAG::D3D11_RESOURCE_MISC_GENERATE_MIPS;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateTexture2D(&textureDesc, nullptr, &texture));

		GetContext(gfx)->UpdateSubresource(texture.Get(), 0U, nullptr, surface.GetBuffer(), static_cast<UINT>(surface.GetRowByteSize()), 0U);

		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.Format = textureDesc.Format;
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipLevels = -1;
		viewDesc.Texture2D.MostDetailedMip = 0;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateShaderResourceView(texture.Get(), &viewDesc, &textureView));

		GetContext(gfx)->GenerateMips(textureView.Get());
		if (alphaEnable)
			checkAlpha->join();
	}
}