#include "GFX/API/DX11/Resource/Texture.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX11::Resource
{
	Texture::Texture(GFX::Device& dev, GFX::Context& ctx, const Surface& surface)
	{
		ZE_GFX_ENABLE_ID(dev.Get().dx11);

		D3D11_TEXTURE2D_DESC textureDesc;
		textureDesc.Width = static_cast<UINT>(surface.GetWidth());
		textureDesc.Height = static_cast<UINT>(surface.GetHeight());
		textureDesc.Format = DX::GetDXFormat(surface.GetFormat());
		textureDesc.MipLevels = 0;
		textureDesc.ArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		D3D11_SUBRESOURCE_DATA resData;
		resData.pSysMem = surface.GetBuffer();
		resData.SysMemPitch = static_cast<UINT>(surface.GetRowByteSize());
		resData.SysMemSlicePitch = 0;

		DX::ComPtr<ID3D11Texture2D> texture;
		ZE_GFX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateTexture2D(&textureDesc, &resData, &texture));

		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
		viewDesc.Format = textureDesc.Format;
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipLevels = -1;
		viewDesc.Texture2D.MostDetailedMip = 0;

		ZE_GFX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateShaderResourceView(texture.Get(), &viewDesc, &srv));
		ctx.Get().dx11.GetContext()->GenerateMips(srv.Get());
		ZE_GFX_SET_ID(srv, "Texture");
	}

	void Texture::BindPS(GFX::Context& ctx, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
		ctx.Get().dx11.GetContext()->PSSetShaderResources(slot, 1, srv.GetAddressOf());
	}

	void Texture::BindCS(GFX::Context& ctx, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
		ctx.Get().dx11.GetContext()->CSSetShaderResources(slot, 1, srv.GetAddressOf());
	}

	Surface Texture::GetData(GFX::Device& dev, GFX::Context& ctx) const
	{
		return { 1, 1 };
	}
}