#include "Texture.h"
#include "GfxExceptionMacros.h"
#include <thread>

namespace GFX::Resource
{
	Texture::Texture(Graphics& gfx, const Surface& surface, const std::string& name, UINT slot, bool alphaEnable) : slot(slot), path(name)
	{
		GFX_ENABLE_ALL(gfx);

		std::unique_ptr<std::thread> checkAlpha = nullptr;
		if (alphaEnable)
			checkAlpha = std::make_unique<std::thread>([&surface, this]() { alpha = surface.HasAlpha(); });

		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		textureDesc.Width = static_cast<UINT>(surface.GetWidth());
		textureDesc.Height = static_cast<UINT>(surface.GetHeight());
		textureDesc.Format = surface.GetFormat();
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
		SET_DEBUG_NAME_RID(textureView.Get());

		GetContext(gfx)->GenerateMips(textureView.Get());
		if (alphaEnable)
			checkAlpha->join();
	}
}