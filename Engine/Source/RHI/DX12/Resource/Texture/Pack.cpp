#include "RHI/DX12/Resource/Texture/Pack.h"

namespace ZE::RHI::DX12::Resource::Texture
{
	Pack::Pack(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::Texture::PackDesc& desc)
	{
		Device& device = dev.Get().dx12;
		ZE_DX_ENABLE_ID(device);

		count = Utils::SafeCast<U32>(desc.Textures.size());
		descInfo = device.AllocDescs(count);
		resources = new ResourceInfo[count];

		std::vector<std::pair<U64, std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>>> copyInfo;
		copyInfo.reserve(count);
		U64 uploadRegionSize = 0;
		// Create destination textures and SRVs for them. Gather info for uploading data
		for (U32 i = 0; const auto & tex : desc.Textures)
		{
			auto& resInfo = resources[i];
			D3D12_SHADER_RESOURCE_VIEW_DESC srv = {};

			copyInfo.emplace_back(0, std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>());
			U16 surfaces = Utils::SafeCast<U16>(tex.Surfaces.size());
			if (surfaces)
			{
				auto& startSurface = tex.Surfaces.front();
				D3D12_RESOURCE_DESC1 texDesc = device.GetTextureDesc(Utils::SafeCast<U32>(startSurface.GetWidth()),
					Utils::SafeCast<U32>(startSurface.GetHeight()), surfaces, srv.Format, tex.Type);

				switch (tex.Type)
				{
				default:
					ZE_ENUM_UNHANDLED();
				case GFX::Resource::Texture::Type::Tex2D:
				{
					D3D12_SUBRESOURCE_FOOTPRINT footprint = {};
					footprint.Format = srv.Format;
					footprint.Width = Utils::SafeCast<U32>(texDesc.Width);
					footprint.Height = texDesc.Height;
					footprint.Depth = 1;
					footprint.RowPitch = Utils::SafeCast<U32>(Math::AlignUp(texDesc.Width * startSurface.GetPixelSize(),
						static_cast<U64>(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)));

					copyInfo.back().second.emplace_back(0, std::move(footprint));
					copyInfo.back().first = Math::AlignUp(Utils::SafeCast<U64>(footprint.Height) * footprint.RowPitch,
						static_cast<U64>(D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT));
					break;
				}
				case GFX::Resource::Texture::Type::Tex3D:
				{
					D3D12_SUBRESOURCE_FOOTPRINT footprint = {};
					footprint.Format = srv.Format;
					footprint.Width = Utils::SafeCast<U32>(texDesc.Width);
					footprint.Height = texDesc.Height;
					footprint.Depth = texDesc.DepthOrArraySize;
					footprint.RowPitch = Utils::SafeCast<U32>(Math::AlignUp(texDesc.Width * startSurface.GetPixelSize(),
						static_cast<U64>(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)));

					copyInfo.back().second.emplace_back(0, std::move(footprint));
					copyInfo.back().first = Math::AlignUp(Utils::SafeCast<U64>(footprint.Height) * footprint.RowPitch,
						static_cast<U64>(D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT)) * texDesc.DepthOrArraySize;
					break;
				}
				case GFX::Resource::Texture::Type::Cube:
				{
					copyInfo.back().second.resize(surfaces);
					device.GetDevice()->GetCopyableFootprints1(&texDesc, 0, 6, 0,
						copyInfo.back().second.data(), nullptr, nullptr, &copyInfo.back().first);
					break;
				}
				case GFX::Resource::Texture::Type::Array:
				{
					copyInfo.back().second.resize(surfaces);
					device.GetDevice()->GetCopyableFootprints1(&texDesc, 0, surfaces, 0,
						copyInfo.back().second.data(), nullptr, nullptr, &copyInfo.back().first);
					break;
				}
				}
				uploadRegionSize += copyInfo.back().first;
			}
		}

		if (uploadRegionSize)
		{
			// Create one big committed buffer and copy data into it
			DX::ComPtr<IResource> uploadRes;// = device.CreateTextureUploadBuffer(uploadRegionSize);
			ZE_DX_SET_ID(uploadRes, "Upload texture buffer: " + std::to_string(uploadRegionSize) + " B");
			D3D12_TEXTURE_COPY_LOCATION copySource = {};
			copySource.pResource = uploadRes.Get();
			copySource.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			U64 bufferOffset = 0;

			D3D12_RANGE range = {};
			U8* uploadBuffer;
			ZE_DX_THROW_FAILED(uploadRes->Map(0, &range, reinterpret_cast<void**>(&uploadBuffer)));
			// Copy all regions upload heap
			for (U32 i = 0; const auto & info : copyInfo)
			{
				auto& tex = desc.Textures.at(i);
				if (tex.Surfaces.size())
				{
					U64 depthSlice = info.first / info.second.size();
					D3D12_TEXTURE_COPY_LOCATION copyDest = {};
					copyDest.pResource = resources[i].Resource.Get();
					copyDest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
					copyDest.SubresourceIndex = 0;

					// Memcpy according to resource structure
					for (U16 j = 0; const auto & region : info.second)
					{
						for (U32 z = 0; z < region.Footprint.Depth; ++z)
						{
							U8* dest = uploadBuffer + z * depthSlice + region.Offset;
							const GFX::Surface& surface = tex.Surfaces.at(Utils::SafeCast<U64>(j) + z);
							const U8* src = reinterpret_cast<const U8*>(surface.GetBuffer());
							for (U32 y = 0; y < region.Footprint.Height; ++y)
							{
								U64 destRowOffset = Utils::SafeCast<U64>(y) * region.Footprint.RowPitch;
								U64 srcRowOffset = Utils::SafeCast<U64>(y) * surface.GetRowByteSize();
								std::memcpy(dest + destRowOffset, src + srcRowOffset, surface.GetRowByteSize());
							}
						}
						copySource.PlacedFootprint.Offset = bufferOffset + region.Offset;
						copySource.PlacedFootprint.Footprint = region.Footprint;

						ZE_ASSERT(tex.Usage != GFX::Resource::Texture::Usage::Invalid, "Texture usage no initialized!");
						D3D12_RESOURCE_STATES endState = D3D12_RESOURCE_STATE_COMMON;
						if (tex.Usage & GFX::Resource::Texture::Usage::PixelShader)
							endState |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
						if (tex.Usage & GFX::Resource::Texture::Usage::NonPixelShader)
							endState |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

						//device.UploadTexture(copyDest, copySource, endState);
						++copyDest.SubresourceIndex;
						++j;
					}
					bufferOffset += info.first;
					uploadBuffer += info.first;
				}
				++i;
			}
			uploadRes->Unmap(0, nullptr);
		}

		for (U32 i = 0; const auto & tex : desc.Textures)
		{
			ZE_ASSERT(tex.Usage != GFX::Resource::Texture::Usage::Invalid, "Texture usage no initialized!");
			ResourceInfo& resInfo = resources[i];

			// Specify default SRV desc
			D3D12_SHADER_RESOURCE_VIEW_DESC srv = {};
			UINT* mipLevels = nullptr;
			srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			switch (tex.Type)
			{
			default:
				ZE_ENUM_UNHANDLED();
			case GFX::Resource::Texture::Type::Tex2D:
			{
				mipLevels = &srv.Texture2D.MipLevels;
				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srv.Texture2D.MostDetailedMip = 0;
				srv.Texture2D.MipLevels = 1;
				srv.Texture2D.PlaneSlice = 0;
				srv.Texture2D.ResourceMinLODClamp = 0.0f;
				break;
			}
			case GFX::Resource::Texture::Type::Tex3D:
			{
				mipLevels = &srv.Texture3D.MipLevels;
				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
				srv.Texture3D.MostDetailedMip = 0;
				srv.Texture3D.MipLevels = 1;
				srv.Texture3D.ResourceMinLODClamp = 0.0f;
				break;
			}
			case GFX::Resource::Texture::Type::Cube:
			{
				mipLevels = &srv.TextureCube.MipLevels;
				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				srv.TextureCube.MostDetailedMip = 0;
				srv.TextureCube.MipLevels = 1;
				srv.TextureCube.ResourceMinLODClamp = 0.0f;
				break;
			}
			case GFX::Resource::Texture::Type::Array:
			{
				mipLevels = &srv.Texture2DArray.MipLevels;
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

			// Check actual provided surface count for given texture
			const U16 surfaces = Utils::SafeCast<U16>(tex.Surfaces.size());
			if (!surfaces)
				srv.Format = DX::GetDXFormat(Settings::BackbufferFormat);
			else
			{
				// Create texture based on format of first surface (other surfaces are constrained to be of same size as previous ones)
				const GFX::Surface& startSurface = tex.Surfaces.front();
				D3D12_RESOURCE_DESC1 texDesc = device.GetTextureDesc(Utils::SafeCast<U32>(startSurface.GetWidth()),
					Utils::SafeCast<U32>(startSurface.GetHeight()), surfaces, srv.Format, tex.Type);

				ZE_ASSERT(texDesc.DepthOrArraySize == 1 || tex.Type != GFX::Resource::Texture::Type::Tex2D, "Single texture cannot hold multiple surfaces!");
				ZE_ASSERT(texDesc.DepthOrArraySize == 6 || tex.Type != GFX::Resource::Texture::Type::Cube, "Cube texture should contain 6 surfaces!");
				if (tex.Type == GFX::Resource::Texture::Type::Array)
					srv.Texture2DArray.ArraySize = texDesc.DepthOrArraySize;
				srv.Format = DX::GetDXFormat(startSurface.GetFormat());
				*mipLevels = texDesc.MipLevels;

				resInfo = device.CreateTexture(texDesc);
				ZE_DX_SET_ID(resInfo.Resource, "Texture_" + std::to_string(i));

				// BIGGER TODO: now all surface data is aligned properly so not copying should be needed. But there can be difference between how
				//   this surfaces are provided. Depth textures will be only from single source (providing multiple should be treated as error)
				//   but arrays of textures can be from multiple surfaces or from single one if DDS texture was loaded. Options:
				//   - check array size of first Surface and act accordingly
				//   - if multiple surfaces provided (they have to contain only single texture with mips) then a) copy them to single buffer
				//     beforehand (or place that burden on user by removing surfaces as vector)
				//     or b) create multiple texture requests if possible. Will have to perform additional
				//     checks during PackDesc creation to see whether there is only single complex array surface or whole vector of them if option b) is chosen.



				// TODO: needed only when computing MIP inside final resource
				// Compute offset of mip levels into final resource
				const U32 rowPitch = Math::AlignUp(Utils::SafeCast<U32>(texDesc.Width) * startSurface.GetPixelSize(), GFX::Surface::ROW_PITCH_ALIGNMENT);
				U32 slicePitch = Math::AlignUp(texDesc.Height * rowPitch, static_cast<U32>(D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT));
				//for (U16 mip = 1; mip < texDesc.MipLevels; ++mip)
				//{
				//	const U32 mipRowPitch = Utils::SafeCast<U32>(Math::AlignUp((texDesc.Width >> mip) * startSurface.GetPixelSize(), Settings::ROW_PITCH_ALIGNMENT));
				//	slicePitch += Math::AlignUp((texDesc.Height >> 1) * mipRowPitch, static_cast<U32>(D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT));
				//}

				// Copy resources according to their alignment requirements
				std::shared_ptr<U8[]> data = std::make_shared<U8[]>(slicePitch * texDesc.DepthOrArraySize);
				for (U16 z = 0; z < texDesc.DepthOrArraySize; ++z)
				{
					const GFX::Surface& surf = tex.Surfaces.at(z);
					const U8* srcBuffer = reinterpret_cast<const U8*>(surf.GetBuffer());
					U8* destBuffer = data.get() + z * slicePitch;

					for (U32 y = 0; y < texDesc.Height; ++y)
						std::memcpy(destBuffer + y * rowPitch, srcBuffer + y * surf.GetRowByteSize(), surf.GetRowByteSize());
				}

				D3D12_RESOURCE_STATES endState = D3D12_RESOURCE_STATE_COMMON;
				if (tex.Usage & GFX::Resource::Texture::Usage::PixelShader)
					endState |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
				if (tex.Usage & GFX::Resource::Texture::Usage::NonPixelShader)
					endState |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

				disk.Get().dx12.AddMemorySingleTextureRequest(INVALID_EID, resInfo.Resource.Get(), std::move(data), slicePitch * texDesc.DepthOrArraySize);
			}

			D3D12_CPU_DESCRIPTOR_HANDLE handle = descInfo.CPU;
			handle.ptr += Utils::SafeCast<U64>(i++) * device.GetDescriptorSize();
			ZE_DX_THROW_FAILED_INFO(device.GetDevice()->CreateShaderResourceView(resInfo.Resource.Get(), &srv, handle));
		}
	}

	Pack::~Pack()
	{
		ZE_ASSERT_FREED(descInfo.Handle == nullptr);
		if (resources)
		{
			for (U32 i = 0; i < count; ++i)
			{
				ZE_ASSERT_FREED(resources[i].IsFree());
			}
			resources.DeleteArray();
		}
	}

	void Pack::Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept
	{
		const auto& schema = bindCtx.BindingSchema.Get().dx12;
		ZE_ASSERT(schema.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::Table,
			"Bind slot is not a descriptor table! Wrong root signature or order of bindings!");

		auto* list = cl.Get().dx12.GetList();
		if (schema.IsCompute())
			list->SetComputeRootDescriptorTable(bindCtx.Count++, descInfo.GPU);
		else
			list->SetGraphicsRootDescriptorTable(bindCtx.Count++, descInfo.GPU);
	}

	void Pack::Free(GFX::Device& dev) noexcept
	{
		if (descInfo.Handle)
			dev.Get().dx12.FreeDescs(descInfo);
		for (U32 i = 0; i < count; ++i)
			if (resources[i].Resource != nullptr)
				dev.Get().dx12.FreeTexture(resources[i]);
	}

	std::vector<std::vector<GFX::Surface>> Pack::GetData(GFX::Device& dev) const
	{
		std::vector<std::vector<GFX::Surface>> vec;
		vec.emplace_back(std::vector<GFX::Surface>());
		vec.front().emplace_back(1, 1);
		return vec;
	}
}