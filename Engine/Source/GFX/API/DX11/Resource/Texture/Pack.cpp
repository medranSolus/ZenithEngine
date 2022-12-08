#include "GFX/API/DX11/Resource/Texture/Pack.h"

namespace ZE::GFX::API::DX11::Resource::Texture
{
	Pack::Pack(GFX::Device& dev, const GFX::Resource::Texture::PackDesc& desc)
	{
		ZE_ASSERT(desc.Textures.size() > 0, "Cannot create empty texture pack!");
		auto& device = dev.Get().dx11;
		ZE_DX_ENABLE_ID(device);

		count = static_cast<U32>(desc.Textures.size());
		srvs = new DX::ComPtr<IShaderResourceView>[count];
		for (U32 i = 0; const auto& tex : desc.Textures)
		{
			if (tex.Surfaces.size() == 0)
				srvs[i] = nullptr;
			else if (tex.Type == GFX::Resource::Texture::Type::Tex3D)
			{
				D3D11_TEXTURE3D_DESC1 texDesc;
				texDesc.Width = static_cast<U32>(tex.Surfaces.front().GetWidth());
				texDesc.Height = static_cast<U32>(tex.Surfaces.front().GetHeight());
				texDesc.Depth = static_cast<U32>(tex.Surfaces.size());
				texDesc.MipLevels = 1;
				texDesc.Format = DX::GetDXFormat(tex.Surfaces.front().GetFormat());
				texDesc.Usage = D3D11_USAGE_IMMUTABLE;
				texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				texDesc.CPUAccessFlags = 0;
				texDesc.MiscFlags = 0;
				texDesc.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;

				D3D11_SUBRESOURCE_DATA* data = new D3D11_SUBRESOURCE_DATA[texDesc.Depth];
				for (U32 j = 0; const auto& surface : tex.Surfaces)
				{
					ZE_ASSERT(DX::GetDXFormat(surface.GetFormat()) == texDesc.Format, "Every surface should have same format!");
					ZE_ASSERT(surface.GetWidth() == texDesc.Width, "Every surface should have same width!");
					ZE_ASSERT(surface.GetHeight() == texDesc.Height, "Every surface should have same height!");

					data[j].pSysMem = surface.GetBuffer();
					data[j].SysMemPitch = static_cast<U32>(surface.GetRowByteSize());
					data[j].SysMemSlicePitch = 0;
					++j;
				}
				DX::ComPtr<ITexture3D> texture;
				ZE_DX_THROW_FAILED(device.GetDevice()->CreateTexture3D1(&texDesc, data, &texture));

				D3D11_SHADER_RESOURCE_VIEW_DESC1 srvDesc;
				srvDesc.Format = texDesc.Format;
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
				srvDesc.Texture3D.MostDetailedMip = 0;
				srvDesc.Texture3D.MipLevels = texDesc.MipLevels;

				ZE_DX_THROW_FAILED(device.GetDevice()->CreateShaderResourceView1(texture.Get(), &srvDesc, &srvs[i]));
				ZE_DX_SET_ID(srvs[i], "Texture3D_" + std::to_string(i));
				delete[] data;
			}
			else
			{
				D3D11_TEXTURE2D_DESC1 texDesc;
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
				texDesc.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;

				D3D11_SHADER_RESOURCE_VIEW_DESC1 srvDesc;
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
						srvDesc.Texture2D.PlaneSlice = 0;
					}
					else
					{
						srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
						srvDesc.Texture2DArray.MostDetailedMip = 0;
						srvDesc.Texture2DArray.MipLevels = texDesc.MipLevels;
						srvDesc.Texture2DArray.FirstArraySlice = 0;
						srvDesc.Texture2DArray.ArraySize = texDesc.ArraySize;
						srvDesc.Texture2DArray.PlaneSlice = 0;
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
					++j;
				}
				DX::ComPtr<ITexture2D> texture;
				ZE_DX_THROW_FAILED(device.GetDevice()->CreateTexture2D1(&texDesc, data, &texture));
				ZE_DX_THROW_FAILED(device.GetDevice()->CreateShaderResourceView1(texture.Get(), &srvDesc, &srvs[i]));
				ZE_DX_SET_ID(srvs[i], "Texture_" + std::to_string(i));
				delete[] data;
			}
			++i;
		}
	}

	Pack::~Pack()
	{
		if (srvs)
		{
			for (U32 i = 0; i < count; ++i)
			{
				ZE_ASSERT(srvs[i] == nullptr, "Resource not freed before deletion!");
			}
			srvs.DeleteArray();
		}
	}

	void Pack::Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept
	{
		auto& schema = bindCtx.BindingSchema.Get().dx11;

		auto slotInfo = schema.GetCurrentSlot(bindCtx.Count++);
		ZE_ASSERT(slotInfo.SlotsCount == 1, "Texture pack slot should only contain 1 entry!");

		auto slotData = schema.GetSlotData(slotInfo.DataStart);
		ZE_ASSERT(slotData.Count == count, "Texture pack slot should contain space for all current textures!");

		auto* ctx = cl.Get().dx11.GetContext();
		for (U32 i = 0; i < count; ++i, ++slotData.BindStart)
		{
			ID3D11ShaderResourceView** srv = reinterpret_cast<ID3D11ShaderResourceView**>(srvs[i].GetAddressOf());
			if (slotData.Shaders & GFX::Resource::ShaderType::Compute)
				ctx->CSSetShaderResources(slotData.BindStart, 1, srv);
			else
			{
				if (slotData.Shaders & GFX::Resource::ShaderType::Vertex)
					ctx->VSSetShaderResources(slotData.BindStart, 1, srv);
				if (slotData.Shaders & GFX::Resource::ShaderType::Domain)
					ctx->DSSetShaderResources(slotData.BindStart, 1, srv);
				if (slotData.Shaders & GFX::Resource::ShaderType::Hull)
					ctx->HSSetShaderResources(slotData.BindStart, 1, srv);
				if (slotData.Shaders & GFX::Resource::ShaderType::Geometry)
					ctx->GSSetShaderResources(slotData.BindStart, 1, srv);
				if (slotData.Shaders & GFX::Resource::ShaderType::Pixel)
					ctx->PSSetShaderResources(slotData.BindStart, 1, srv);
			}
		}
	}

	void Pack::Free(GFX::Device& dev) noexcept
	{
		for (U32 i = 0; i < count; ++i)
			srvs[i] = nullptr;
	}

	std::vector<std::vector<Surface>> Pack::GetData(GFX::Device& dev) const
	{
		std::vector<std::vector<Surface>> vec;
		vec.emplace_back(std::vector<Surface>());
		vec.front().emplace_back(1U, 1U);
		return vec;
	}
}