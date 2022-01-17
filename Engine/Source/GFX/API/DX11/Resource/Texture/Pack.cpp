#include "GFX/API/DX11/Resource/Texture/Pack.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX11::Resource::Texture
{
	Pack::Pack(GFX::Device& dev, const GFX::Resource::Texture::PackDesc& desc)
	{
		ZE_ASSERT(desc.Textures.size() > 0, "Cannot create empty texture pack!");
		auto& device = dev.Get().dx11;
		ZE_GFX_ENABLE_ID(device);

		count = static_cast<U32>(desc.Textures.size());
		srvs = new DX::ComPtr<ID3D11ShaderResourceView>[count];
		for (U32 i = 0; const auto& tex : desc.Textures)
		{
			if (tex.Surfaces.size() == 0)
				srvs[i] = nullptr;
			else if (tex.Type == GFX::Resource::Texture::Type::Tex3D)
			{
				D3D11_TEXTURE3D_DESC texDesc;
				texDesc.Width = static_cast<U32>(tex.Surfaces.front().GetWidth());
				texDesc.Height = static_cast<U32>(tex.Surfaces.front().GetHeight());
				texDesc.Depth = static_cast<U32>(tex.Surfaces.size());
				texDesc.MipLevels = 1;
				texDesc.Format = DX::GetDXFormat(tex.Surfaces.front().GetFormat());
				texDesc.Usage = D3D11_USAGE_IMMUTABLE;
				texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				texDesc.CPUAccessFlags = 0;
				texDesc.MiscFlags = 0;

				D3D11_SUBRESOURCE_DATA* data = new D3D11_SUBRESOURCE_DATA[texDesc.Depth];
				for (U32 j = 0; const auto& surface : tex.Surfaces)
				{
					ZE_ASSERT(DX::GetDXFormat(surface.GetFormat()) == texDesc.Format, "Every surface should have same format!");
					ZE_ASSERT(surface.GetWidth() == texDesc.Width, "Every surface should have same width!");
					ZE_ASSERT(surface.GetHeight() == texDesc.Height, "Every surface should have same height!");

					data[j].pSysMem = surface.GetBuffer();
					data[j].SysMemPitch = static_cast<U32>(surface.GetRowByteSize());
					data[j].SysMemSlicePitch = 0;
				}
				Microsoft::WRL::ComPtr<ID3D11Texture3D> texture;
				ZE_GFX_THROW_FAILED(device.GetDevice()->CreateTexture3D(&texDesc, data, &texture));

				D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
				srvDesc.Format = texDesc.Format;
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
				srvDesc.Texture3D.MostDetailedMip = 0;
				srvDesc.Texture3D.MipLevels = texDesc.MipLevels;

				ZE_GFX_THROW_FAILED(device.GetDevice()->CreateShaderResourceView(texture.Get(), &srvDesc, &srvs[i]));
				ZE_GFX_SET_ID(srvs[i], "Texture3D_" + std::to_string(i));
				delete[] data;
			}
			else
			{
				D3D11_TEXTURE2D_DESC texDesc;
				texDesc.Width = static_cast<U32>(tex.Surfaces.front().GetWidth());
				texDesc.Height = static_cast<U32>(tex.Surfaces.front().GetHeight());
				texDesc.MipLevels = 1; // TODO: Add mip generation module
				texDesc.ArraySize = static_cast<U32>(tex.Surfaces.size());
				texDesc.Format = DX::GetDXFormat(tex.Surfaces.front().GetFormat());
				texDesc.SampleDesc.Count = 1;
				texDesc.SampleDesc.Quality = 0;
				texDesc.Usage = D3D11_USAGE_IMMUTABLE;
				texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				texDesc.CPUAccessFlags = 0;

				D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
				srvDesc.Format = texDesc.Format;
				if (tex.Type == GFX::Resource::Texture::Type::Cube)
				{
					ZE_ASSERT(texDesc.ArraySize == 6, "Cube texture should contain 6 surfaces!");
					texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
					srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
					srvDesc.TextureCube.MostDetailedMip = 0;
					srvDesc.TextureCube.MipLevels = texDesc.MipLevels;
				}
				else
				{
					texDesc.MiscFlags = 0;
					if (tex.Type == GFX::Resource::Texture::Type::Tex2D)
					{
						ZE_ASSERT(texDesc.ArraySize == 1, "Single texture cannot hold multiple surfaces!");
						srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
						srvDesc.Texture2D.MostDetailedMip = 0;
						srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
					}
					else
					{
						srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
						srvDesc.Texture2DArray.MostDetailedMip = 0;
						srvDesc.Texture2DArray.MipLevels = texDesc.MipLevels;
						srvDesc.Texture2DArray.FirstArraySlice = 0;
						srvDesc.Texture2DArray.ArraySize = texDesc.ArraySize;
					}
				}

				D3D11_SUBRESOURCE_DATA* data = new D3D11_SUBRESOURCE_DATA[texDesc.ArraySize];
				for (U32 j = 0; const auto & surface : tex.Surfaces)
				{
					ZE_ASSERT(DX::GetDXFormat(surface.GetFormat()) == texDesc.Format, "Every surface should have same format!");
					ZE_ASSERT(surface.GetWidth() == texDesc.Width, "Every surface should have same width!");
					ZE_ASSERT(surface.GetHeight() == texDesc.Height, "Every surface should have same height!");

					data[j].pSysMem = surface.GetBuffer();
					data[j].SysMemPitch = static_cast<U32>(surface.GetRowByteSize());
					data[j].SysMemSlicePitch = 0;
				}
				DX::ComPtr<ID3D11Texture2D> texture;
				ZE_GFX_THROW_FAILED(device.GetDevice()->CreateTexture2D(&texDesc, data, &texture));
				ZE_GFX_THROW_FAILED(device.GetDevice()->CreateShaderResourceView(texture.Get(), &srvDesc, &srvs[i]));
				ZE_GFX_SET_ID(srvs[i], "Texture_" + std::to_string(i));
				delete[] data;
			}
			++i;
		}
	}

	Pack::~Pack()
	{
		if (srvs)
			srvs.DeleteArray();
	}

	void Pack::Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept
	{
	}

	std::vector<std::vector<Surface>> Pack::GetData(GFX::Device& dev) const
	{
		std::vector<std::vector<Surface>> vec;
		vec.emplace_back(std::vector<Surface>());
		vec.front().emplace_back(1, 1);
		return vec;
	}
}