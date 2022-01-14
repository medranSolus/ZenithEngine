#include "GFX/API/DX12/Resource/TexturePack.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX12::Resource
{
	TexturePack::TexturePack(GFX::Device& dev, const GFX::Resource::TexturePackDesc& desc)
	{
		ZE_ASSERT(desc.Textures.size() > 0, "Cannot create empty texture pack!");
		auto& device = dev.Get().dx12;
		ZE_GFX_ENABLE_ID(device);

		count = static_cast<U32>(desc.Textures.size());
		descInfo = device.AllocDescs(count);
		resources = new ResourceInfo[count];

		std::vector<std::pair<U64, std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>>> copyInfo;
		copyInfo.reserve(count);
		U64 uploadRegionSize = 0;
		// Create destination textures and SRVs for them. Gather info for uploading data
		for (U32 i = 0; const auto& tex : desc.Textures)
		{
			auto& resInfo = resources[i++];
			D3D12_SHADER_RESOURCE_VIEW_DESC srv;
			srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			switch (tex.Type)
			{
			case GFX::Resource::TextureType::Tex2D:
			{
				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srv.Texture2D.MostDetailedMip = 0;
				srv.Texture2D.MipLevels = 1;
				srv.Texture2D.PlaneSlice = 0;
				srv.Texture2D.ResourceMinLODClamp = 0.0f;
				break;
			}
			case GFX::Resource::TextureType::Tex3D:
			{
				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
				srv.Texture3D.MostDetailedMip = 0;
				srv.Texture3D.MipLevels = 1;
				srv.Texture3D.ResourceMinLODClamp = 0.0f;
				break;
			}
			case GFX::Resource::TextureType::Cube:
			{
				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				srv.TextureCube.MostDetailedMip = 0;
				srv.TextureCube.MipLevels = 1;
				srv.TextureCube.ResourceMinLODClamp = 0.0f;
				break;
			}
			case GFX::Resource::TextureType::Array:
			{
				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				srv.Texture2DArray.MostDetailedMip = 0;
				srv.Texture2DArray.MipLevels = 1;
				srv.Texture2DArray.FirstArraySlice = 0;
				srv.Texture2DArray.ArraySize = 1;
				srv.Texture2DArray.PlaneSlice = 0;
				srv.Texture2DArray.ResourceMinLODClamp = 0.0f;
				break;
			}
			}

			copyInfo.emplace_back(0, std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>());
			U16 surfaces = static_cast<U16>(tex.Surfaces.size());
			if (tex.Surfaces.size())
			{
				auto& startSurface = tex.Surfaces.front();
				srv.Format = DX::GetDXFormat(startSurface.GetFormat());
				auto texDesc = device.GetTextureDesc(static_cast<U32>(tex.Surfaces.front().GetWidth()),
					static_cast<U32>(tex.Surfaces.front().GetHeight()), surfaces,
					srv.Format, tex.Type == GFX::Resource::TextureType::Tex3D);

				switch (tex.Type)
				{
				case GFX::Resource::TextureType::Tex2D:
				{
					ZE_ASSERT(texDesc.first.DepthOrArraySize == 1, "Single texture cannot hold multiple surfaces!");
					srv.Texture2D.MipLevels = texDesc.first.MipLevels;

					D3D12_SUBRESOURCE_FOOTPRINT footprint;
					footprint.Format = srv.Format;
					footprint.Width = static_cast<U32>(texDesc.first.Width);
					footprint.Height = texDesc.first.Height;
					footprint.Depth = 1;
					footprint.RowPitch = static_cast<U32>(startSurface.GetRowByteSize());

					copyInfo.back().second.emplace_back(0, std::move(footprint));
					copyInfo.back().first = startSurface.GetSliceByteSize();
					break;
				}
				case GFX::Resource::TextureType::Tex3D:
				{
					srv.Texture3D.MipLevels = texDesc.first.MipLevels;

					D3D12_SUBRESOURCE_FOOTPRINT footprint;
					footprint.Format = srv.Format;
					footprint.Width = static_cast<U32>(texDesc.first.Width);
					footprint.Height = texDesc.first.Height;
					footprint.Depth = texDesc.first.DepthOrArraySize;
					footprint.RowPitch = static_cast<U32>(startSurface.GetRowByteSize());

					copyInfo.back().second.emplace_back(0, std::move(footprint));
					copyInfo.back().first = startSurface.GetSliceByteSize() * texDesc.first.DepthOrArraySize;
					break;
				}
				case GFX::Resource::TextureType::Cube:
				{
					ZE_ASSERT(texDesc.first.DepthOrArraySize == 6, "Cube texture should contain 6 surfaces!");
					srv.TextureCube.MipLevels = texDesc.first.MipLevels;

					copyInfo.back().second.resize(surfaces);
					device.GetDevice()->GetCopyableFootprints(&texDesc.first, 0, 6, 0,
						copyInfo.back().second.data(), nullptr, nullptr, &copyInfo.back().first);
					break;
				}
				case GFX::Resource::TextureType::Array:
				{
					srv.Texture2DArray.MipLevels = texDesc.first.MipLevels;
					srv.Texture2DArray.ArraySize = texDesc.first.DepthOrArraySize;
					copyInfo.back().second.resize(surfaces);
					device.GetDevice()->GetCopyableFootprints(&texDesc.first, 0, surfaces, 0,
						copyInfo.back().second.data(), nullptr, nullptr, &copyInfo.back().first);
					break;
				}
				}
				resInfo = device.CreateTexture(texDesc);
				ZE_GFX_SET_ID(resInfo.Resource, "Texture_" + std::to_string(i));
				uploadRegionSize += copyInfo.back().first;
			}
			else
				srv.Format = DX::GetDXFormat(Settings::GetBackbufferFormat());

			D3D12_CPU_DESCRIPTOR_HANDLE handle = descInfo.CPU;
			handle.ptr += static_cast<U64>(i) * device.GetDescriptorSize();
			device.GetDevice()->CreateShaderResourceView(resInfo.Resource.Get(), &srv, handle);
		}

		// Create one big committed buffer and copy data into it
		DX::ComPtr<ID3D12Resource> uploadRes = device.CreateTextureUploadBuffer(uploadRegionSize);
		ZE_GFX_SET_ID(uploadRes, "Upload texture buffer: " + std::to_string(uploadRegionSize) + " B");
		D3D12_TEXTURE_COPY_LOCATION copySource;
		copySource.pResource = uploadRes.Get();
		copySource.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		U64 bufferOffset = 0;

		D3D12_RANGE range = { 0 };
		U8* uploadBuffer;
		ZE_GFX_THROW_FAILED(uploadRes->Map(0, &range, reinterpret_cast<void**>(&uploadBuffer)));
		// Copy all regions upload heap
		for (U32 i = 0; const auto& info : copyInfo)
		{
			auto& tex = desc.Textures.at(i);
			U64 depthSlice = info.first / info.second.size();
			D3D12_TEXTURE_COPY_LOCATION copyDest;
			copyDest.pResource = resources[i].Resource.Get();
			copyDest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			copyDest.SubresourceIndex = 0;

			// Memcpy according to resource structure
			for (U16 j = 0; const auto& region : info.second)
			{
				for (U32 z = 0; z < region.Footprint.Depth; ++z)
				{
					U8* dest = uploadBuffer + z * depthSlice + region.Offset;
					const U8* src = reinterpret_cast<const U8*>(tex.Surfaces.at(static_cast<U64>(j) + z).GetBuffer());
					for (U32 y = 0; y < region.Footprint.Height; ++y)
					{
						U64 rowOffset = static_cast<U64>(y) * region.Footprint.RowPitch;
						memcpy(dest + rowOffset, src + rowOffset, region.Footprint.RowPitch);
					}
				}
				copySource.PlacedFootprint.Offset = bufferOffset + region.Offset;
				copySource.PlacedFootprint.Footprint = region.Footprint;

				ZE_ASSERT(tex.Usage != GFX::Resource::TextureUsage::Invalid, "Texture usage no initialized!");
				D3D12_RESOURCE_STATES endState = D3D12_RESOURCE_STATE_COMMON;
				if (tex.Usage & GFX::Resource::TextureUsage::PixelShader)
					endState |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
				if (tex.Usage & GFX::Resource::TextureUsage::NonPixelShader)
					endState |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

				device.UploadTexture(copyDest, copySource, endState);
				++copyDest.SubresourceIndex;
				++j;
			}
			bufferOffset += info.first;
			uploadBuffer += info.first;
			++i;
		}
		uploadRes->Unmap(0, nullptr);
	}

	TexturePack::~TexturePack()
	{
		if (resources)
			delete[] resources;
	}

	void TexturePack::Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept
	{
		assert(0 < D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
	}

	std::vector<std::vector<Surface>> TexturePack::GetData(GFX::Device& dev) const
	{
		std::vector<std::vector<Surface>> vec;
		vec.emplace_back(std::vector<Surface>());
		vec.front().emplace_back(1, 1);
		return vec;
	}
}