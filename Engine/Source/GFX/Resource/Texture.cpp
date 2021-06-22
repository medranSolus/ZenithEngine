#include "GFX/Resource/Texture.h"
#include "GFX/Resource/GfxDebugName.h"
#include "GFX/Graphics.h"
#include <thread>

namespace ZE::GFX::Resource
{
	Texture::Texture(Graphics& gfx, const Surface& surface, const std::string& name, U32 slot, bool alphaEnable) : slot(slot), path(name)
	{
		assert(slot < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
		ZE_GFX_ENABLE_ALL(gfx);

		std::unique_ptr<std::thread> checkAlpha = nullptr;
		if (alphaEnable)
			checkAlpha = std::make_unique<std::thread>([&surface, this]() { alpha = surface.HasAlpha(); });

		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		textureDesc.Width = static_cast<UINT>(surface.GetWidth());
		textureDesc.Height = static_cast<UINT>(surface.GetHeight());
		textureDesc.Format = API::GetDXFormat(surface.GetFormat());
		textureDesc.MipLevels = 0;
		textureDesc.ArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateTexture2D(&textureDesc, nullptr, &texture));

		GetContext(gfx)->UpdateSubresource(texture.Get(), 0, nullptr, surface.GetBuffer(), static_cast<UINT>(surface.GetRowByteSize()), 0);

		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.Format = textureDesc.Format;
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipLevels = -1;
		viewDesc.Texture2D.MostDetailedMip = 0;
		ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateShaderResourceView(texture.Get(), &viewDesc, &textureView));
		ZE_GFX_SET_RID(textureView.Get());

		GetContext(gfx)->GenerateMips(textureView.Get());
		if (alphaEnable)
			checkAlpha->join();
	}

	std::string Texture::GenerateRID(const std::string& path, U32 slot, bool alphaEnable) noexcept
	{
		return "T" + std::to_string(slot) + "#" + path;
	}

	std::string Texture::GenerateRID(const Surface& surface, const std::string& name, U32 slot, bool alphaEnable) noexcept
	{
		return GenerateRID(name, slot, alphaEnable);
	}

	GfxResPtr<Texture> Texture::Get(Graphics& gfx, const std::string& path, U32 slot, bool alphaEnable)
	{
		return Codex::Resolve<Texture>(gfx, Surface(path), path, slot, alphaEnable);
	}

	GfxResPtr<Texture> Texture::Get(Graphics& gfx, const Surface& surface, const std::string& name, U32 slot, bool alphaEnable)
	{
		return Codex::Resolve<Texture>(gfx, surface, name, slot, alphaEnable);
	}
}