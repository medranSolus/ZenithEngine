#include "RHI/DX11/Resource/Texture/Pack.h"

namespace ZE::RHI::DX11::Resource::Texture
{
	static Expected<DX::ComPtr<IResource>> CreateTexture(IDevice* device, D3D11_SHADER_RESOURCE_VIEW_DESC1& srvDesc, D3D11_SUBRESOURCE_DATA* resData,
		GFX::Resource::Texture::Type type, U32 width, U32 height, U32 mips, U32 depthOrArraySize, D3D11_USAGE usage) noexcept
	{
		switch (type)
		{
		case GFX::Resource::Texture::Type::Tex1D:
		case GFX::Resource::Texture::Type::Tex1DArray:
		{
			ZE_ASSERT(height == 1, "1D texture should have height of 1!");

			D3D11_TEXTURE1D_DESC texDesc = {};
			texDesc.Width = width;
			texDesc.MipLevels = mips;
			texDesc.ArraySize = depthOrArraySize;
			texDesc.Format = srvDesc.Format;
			texDesc.Usage = usage;
			texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			texDesc.CPUAccessFlags = 0;
			texDesc.MiscFlags = 0;

			if (type == GFX::Resource::Texture::Type::Tex1D)
			{
				ZE_ASSERT(texDesc.ArraySize == 1, "Single texture cannot hold multiple surfaces!");
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
				srvDesc.Texture1D.MostDetailedMip = 0;
				srvDesc.Texture1D.MipLevels = texDesc.MipLevels;
			}
			else
			{
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
				srvDesc.Texture1DArray.MostDetailedMip = 0;
				srvDesc.Texture1DArray.MipLevels = texDesc.MipLevels;
				srvDesc.Texture1DArray.FirstArraySlice = 0;
				srvDesc.Texture1DArray.ArraySize = texDesc.ArraySize;
			}

			DX::ComPtr<ITexture1D> texture;
			ZE_DX_RET_FAILED_EXPECT(device->CreateTexture1D(&texDesc, resData, &texture));
			return texture;
		}
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::Texture::Type::Tex2D:
		case GFX::Resource::Texture::Type::Tex2DArray:
		case GFX::Resource::Texture::Type::Cube:
		{
			D3D11_TEXTURE2D_DESC1 texDesc = {};
			texDesc.Width = width;
			texDesc.Height = height;
			texDesc.MipLevels = mips;
			texDesc.ArraySize = depthOrArraySize;
			texDesc.Format = srvDesc.Format;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			texDesc.Usage = usage;
			texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			texDesc.CPUAccessFlags = 0;
			texDesc.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;

			if (type == GFX::Resource::Texture::Type::Cube)
			{
				ZE_ASSERT(texDesc.ArraySize % 6 == 0, "Cube texture should contain multiple of 6 surfaces!");
				texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
				if (texDesc.ArraySize > 6)
				{
					srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
					srvDesc.TextureCubeArray.MostDetailedMip = 0;
					srvDesc.TextureCubeArray.MipLevels = texDesc.MipLevels;
					srvDesc.TextureCubeArray.First2DArrayFace = 0;
					srvDesc.TextureCubeArray.NumCubes = texDesc.ArraySize / 6;
				}
				else
				{
					srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
					srvDesc.TextureCube.MostDetailedMip = 0;
					srvDesc.TextureCube.MipLevels = texDesc.MipLevels;
				}
			}
			else
			{
				texDesc.MiscFlags = 0;
				if (type == GFX::Resource::Texture::Type::Tex2D)
				{
					ZE_ASSERT(texDesc.ArraySize == 1, "Single texture cannot hold multiple surfaces!");
					srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
					srvDesc.Texture2D.MostDetailedMip = 0;
					srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
					srvDesc.Texture2D.PlaneSlice = 0;
				}
				else
				{
					srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
					srvDesc.Texture2DArray.MostDetailedMip = 0;
					srvDesc.Texture2DArray.MipLevels = texDesc.MipLevels;
					srvDesc.Texture2DArray.FirstArraySlice = 0;
					srvDesc.Texture2DArray.ArraySize = texDesc.ArraySize;
					srvDesc.Texture2DArray.PlaneSlice = 0;
				}
			}

			DX::ComPtr<ITexture2D> texture;
			ZE_DX_RET_FAILED_EXPECT(device->CreateTexture2D1(&texDesc, resData, &texture));
			return texture;
		}
		case GFX::Resource::Texture::Type::Tex3D:
		{
			D3D11_TEXTURE3D_DESC1 texDesc = {};
			texDesc.Width = width;
			texDesc.Height = height;
			texDesc.Depth = depthOrArraySize;
			texDesc.MipLevels = mips;
			texDesc.Format = srvDesc.Format;
			texDesc.Usage = usage;
			texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			texDesc.CPUAccessFlags = 0;
			texDesc.MiscFlags = 0;
			texDesc.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;

			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
			srvDesc.Texture3D.MostDetailedMip = 0;
			srvDesc.Texture3D.MipLevels = texDesc.MipLevels;

			DX::ComPtr<ITexture3D> texture;
			ZE_DX_RET_FAILED_EXPECT(device->CreateTexture3D1(&texDesc, resData, &texture));
			return texture;
		}
		}
	}

	Expected<Pack> Pack::Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::Texture::PackDesc& desc) noexcept
	{
		auto* device = dev.Get().dx11.GetDevice();

		Pack pack = {};
		pack.count = Utils::SafeCast<U32>(desc.Textures.size());
		pack.srvs = std::make_unique_for_overwrite<DX::ComPtr<IShaderResourceView>[]>(pack.count);

		for (U32 i = 0; const auto& tex : desc.Textures)
		{
			const U32 surfaces = Utils::SafeCast<U32>(tex.Surfaces.size());
			if (surfaces == 0)
				pack.srvs[i] = nullptr;
			else
			{
				// Gather source data
				const GFX::Surface& startSurface = tex.Surfaces.front();
				auto resData = std::make_unique<D3D11_SUBRESOURCE_DATA[]>(surfaces);
				for (U32 j = 0; const auto& surface : tex.Surfaces)
				{
					ZE_ASSERT(surface.GetFormat() == startSurface.GetFormat(), "Every surface should have same format!");
					ZE_ASSERT(surface.GetWidth() == startSurface.GetWidth(), "Every surface should have same width!");
					ZE_ASSERT(surface.GetHeight() == startSurface.GetHeight(), "Every surface should have same height!");

					resData[j].pSysMem = surface.GetBuffer();
					resData[j].SysMemPitch = surface.GetRowByteSize();
					resData[j].SysMemSlicePitch = 0;
					++j;
				}

				D3D11_SHADER_RESOURCE_VIEW_DESC1 srvDesc = {};
				srvDesc.Format = DX::GetDXFormat(startSurface.GetFormat());

				DX::ComPtr<IResource> resource;
				ZE_EXPECT_RET_FAILED(resource, CreateTexture(device, srvDesc, resData.get(),
					tex.Type, startSurface.GetWidth(), startSurface.GetHeight(), startSurface.GetMipCount(),
					startSurface.GetDepth() > 1 ? startSurface.GetDepth() : (surfaces > 1 ? surfaces : startSurface.GetArraySize()),
					D3D11_USAGE_IMMUTABLE));
				ZE_DX_SET_ID(resource, "Texture_" + std::to_string(i) + "_ID_" + std::to_string(static_cast<U64>(desc.ResourceID)) + (desc.DebugName.size() ? "_" + desc.DebugName : ""));

				ZE_DX_RET_FAILED_EXPECT(device->CreateShaderResourceView1(resource.Get(), &srvDesc, &pack.srvs[i]));
				ZE_DX_SET_ID(pack.srvs[i], "TextureSRV_" + std::to_string(i) + "_ID_" + std::to_string(static_cast<U64>(desc.ResourceID)) + (desc.DebugName.size() ? "_" + desc.DebugName : ""));
			}
			++i;
		}
		return pack;
	}

	Expected<Pack> Pack::Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::Texture::PackFileDesc& desc, GFX::GFile& file) noexcept
	{
		auto* device = dev.Get().dx11.GetDevice();

		Pack pack = {};
		pack.count = Utils::SafeCast<U32>(desc.Textures.size());
		pack.srvs = std::make_unique_for_overwrite<DX::ComPtr<IShaderResourceView>[]>(pack.count);

		for (U32 i = 0; const auto& tex : desc.Textures)
		{
			if (tex.Format == PixelFormat::Unknown)
				pack.srvs[i] = nullptr;
			else
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC1 srvDesc = {};
				srvDesc.Format = DX::GetDXFormat(tex.Format);

				DX::ComPtr<IResource> resource;
				ZE_EXPECT_RET_FAILED(resource, CreateTexture(device, srvDesc, nullptr,
					tex.Type, tex.Width, tex.Height, tex.MipLevels, tex.DepthArraySize, D3D11_USAGE_DEFAULT));
				ZE_DX_SET_ID(resource, "Texture_" + std::to_string(i) + "_ID_" + std::to_string(static_cast<U64>(desc.ResourceID)));

				ZE_DX_RET_FAILED_EXPECT(device->CreateShaderResourceView1(resource.Get(), &srvDesc, &pack.srvs[i]));
				ZE_DX_SET_ID(pack.srvs[i], "TextureSRV_" + std::to_string(i) + "_ID_" + std::to_string(static_cast<U64>(desc.ResourceID)));
			
				disk.Get().dx11.AddFileTextureRequest(resource, file, tex.DataOffset, tex.SourceBytes, tex.Compression, tex.UncompressedSize);
			}
			++i;
		}
		disk.Get().dx11.AddTexturePackID(desc.ResourceID);
		return pack;
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
}