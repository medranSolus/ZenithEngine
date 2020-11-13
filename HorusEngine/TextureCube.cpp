#include "TextureCube.h"
#include "Surface.h"
#include "GfxExceptionMacros.h"

namespace GFX::Resource
{
	TextureCube::TextureCube(Graphics& gfx, const std::string& path, const std::string& ext, UINT slot)
		: slot(slot), path(path), ext(ext)
	{
		GFX_ENABLE_ALL(gfx);

		std::vector<Surface> surfaces;
		surfaces.reserve(6);
		surfaces.emplace_back(path + "\\px" + ext); // Right
		surfaces.emplace_back(path + "\\nx" + ext); // Left
		surfaces.emplace_back(path + "\\py" + ext); // Up
		surfaces.emplace_back(path + "\\ny" + ext); // Down
		surfaces.emplace_back(path + "\\pz" + ext); // Front
		surfaces.emplace_back(path + "\\nz" + ext); // Back

		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		textureDesc.Width = static_cast<UINT>(surfaces.at(0).GetWidth());
		textureDesc.Height = static_cast<UINT>(surfaces.at(0).GetHeight());
		textureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 6;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_FLAG::D3D11_RESOURCE_MISC_TEXTURECUBE;

		D3D11_SUBRESOURCE_DATA data[6];
		for (unsigned char i = 0; i < 6; ++i)
		{
			data[i].pSysMem = surfaces.at(i).GetBuffer();
			data[i].SysMemPitch = static_cast<UINT>(surfaces.at(i).GetRowByteSize());
			data[i].SysMemSlicePitch = 0U;
		}
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateTexture2D(&textureDesc, data, &texture));

		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.Format = textureDesc.Format;
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURECUBE;
		viewDesc.Texture2D.MipLevels = 1;
		viewDesc.Texture2D.MostDetailedMip = 0;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateShaderResourceView(texture.Get(), &viewDesc, &textureView));
		SET_DEBUG_NAME_RID(textureView.Get());
	}
}