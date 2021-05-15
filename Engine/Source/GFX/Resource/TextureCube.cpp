#include "GFX/Resource/TextureCube.h"
#include "GFX/Resource/GfxDebugName.h"
#include "GFX/Graphics.h"

namespace ZE::GFX::Resource
{
	void TextureCube::SetTexture(Graphics& gfx, const std::string& dir, const std::string& fileExt)
	{
		ZE_GFX_ENABLE_ALL(gfx);

		std::vector<Surface> surfaces;
		surfaces.reserve(6);
		surfaces.emplace_back(dir + "\\px" + fileExt); // Right
		surfaces.emplace_back(dir + "\\nx" + fileExt); // Left
		surfaces.emplace_back(dir + "\\py" + fileExt); // Up
		surfaces.emplace_back(dir + "\\ny" + fileExt); // Down
		surfaces.emplace_back(dir + "\\pz" + fileExt); // Front
		surfaces.emplace_back(dir + "\\nz" + fileExt); // Back
		path = dir;
		ext = fileExt;

		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		textureDesc.Width = static_cast<UINT>(surfaces.at(0).GetWidth());
		textureDesc.Height = static_cast<UINT>(surfaces.at(0).GetHeight());
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 6;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		D3D11_SUBRESOURCE_DATA data[6];
		for (U8 i = 0; i < 6; ++i)
		{
			data[i].pSysMem = surfaces.at(i).GetBuffer();
			data[i].SysMemPitch = static_cast<UINT>(surfaces.at(i).GetRowByteSize());
			data[i].SysMemSlicePitch = 0;
		}
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateTexture2D(&textureDesc, data, &texture));

		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.Format = textureDesc.Format;
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURECUBE;
		viewDesc.Texture2D.MipLevels = 1;
		viewDesc.Texture2D.MostDetailedMip = 0;
		ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateShaderResourceView(texture.Get(), &viewDesc, &textureView));
		ZE_GFX_SET_RID(textureView.Get());
	}

	std::string TextureCube::GenerateRID(const std::string& path, const std::string& ext, U32 slot) noexcept
	{
		return "C" + std::to_string(slot) + "#" + path + ext;
	}

	GfxResPtr<TextureCube> TextureCube::Get(Graphics& gfx, const std::string& path, const std::string& ext, U32 slot)
	{
		return Codex::Resolve<TextureCube>(gfx, path, ext, slot);
	}
}