#include "GFX/API/DX11/Resource/TextureCube.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX11::Resource
{
	TextureCube::TextureCube(GFX::Device& dev, const std::array<Surface, 6>& surfaces)
	{
		ZE_GFX_ENABLE_ID(dev.Get().dx11);

		D3D11_TEXTURE2D_DESC textureDesc;
		textureDesc.Width = static_cast<UINT>(surfaces[0].GetWidth());
		textureDesc.Height = static_cast<UINT>(surfaces[0].GetHeight());
		textureDesc.Format = DX::GetDXFormat(surfaces[0].GetFormat());;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 6;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		D3D11_SUBRESOURCE_DATA data[6];
		for (U8 i = 0; i < 6; ++i)
		{
			data[i].pSysMem = surfaces[i].GetBuffer();
			data[i].SysMemPitch = static_cast<UINT>(surfaces[i].GetRowByteSize());
			data[i].SysMemSlicePitch = 0;
		}
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		ZE_GFX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateTexture2D(&textureDesc, data, &texture));

		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
		viewDesc.Format = textureDesc.Format;
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		viewDesc.Texture2D.MipLevels = 1;

		ZE_GFX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateShaderResourceView(texture.Get(), &viewDesc, &srv));
		ZE_GFX_SET_ID(srv, "TextureCube");
	}

	void TextureCube::BindPS(GFX::Context& ctx, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
		ctx.Get().dx11.GetContext()->PSSetShaderResources(slot, 1, srv.GetAddressOf());
	}

	void TextureCube::BindCS(GFX::Context& ctx, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
		ctx.Get().dx11.GetContext()->CSSetShaderResources(slot, 1, srv.GetAddressOf());
	}

	std::array<Surface, 6> TextureCube::GetData(GFX::Device& dev, GFX::Context& ctx) const
	{
		return
		{
			Surface(1, 1),
			Surface(1, 1),
			Surface(1, 1),
			Surface(1, 1),
			Surface(1, 1),
			Surface(1, 1)
		};
	}
}