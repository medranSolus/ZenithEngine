#include "RHI/DX12/Resource/Texture/Pack.h"

namespace ZE::RHI::DX12::Resource::Texture
{
	Pack::Pack(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::Texture::PackDesc& desc)
	{
		Device& device = dev.Get().dx12;
		DiskManager& diskManager = disk.Get().dx12;
		ZE_DX_ENABLE_ID(device);

		count = Utils::SafeCast<U32>(desc.Textures.size());
		descInfo = device.AllocDescs(count);
		resources = new ResourceInfo[count];

		for (U32 i = 0; const auto& tex : desc.Textures)
		{
			ResourceInfo& resInfo = resources[i];

			// Specify default SRV desc
			D3D12_SHADER_RESOURCE_VIEW_DESC srv = {};
			UINT* mipLevels = nullptr;
			srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			switch (tex.Type)
			{
			case GFX::Resource::Texture::Type::Tex1D:
			{
				mipLevels = &srv.Texture1D.MipLevels;
				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
				srv.Texture1D.MostDetailedMip = 0;
				srv.Texture1D.ResourceMinLODClamp = 0.0f;
				break;
			}
			case GFX::Resource::Texture::Type::Tex1DArray:
			{
				mipLevels = &srv.Texture1DArray.MipLevels;
				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
				srv.Texture1DArray.MostDetailedMip = 0;
				srv.Texture1DArray.FirstArraySlice = 0;
				srv.Texture1DArray.ArraySize = 1;
				srv.Texture1DArray.ResourceMinLODClamp = 0.0f;
				break;
			}
			default:
				ZE_ENUM_UNHANDLED();
			case GFX::Resource::Texture::Type::Tex2D:
			{
				mipLevels = &srv.Texture2D.MipLevels;
				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srv.Texture2D.MostDetailedMip = 0;
				srv.Texture2D.PlaneSlice = 0;
				srv.Texture2D.ResourceMinLODClamp = 0.0f;
				break;
			}
			case GFX::Resource::Texture::Type::Tex2DArray:
			{
				mipLevels = &srv.Texture2DArray.MipLevels;
				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				srv.Texture2DArray.MostDetailedMip = 0;
				srv.Texture2DArray.FirstArraySlice = 0;
				srv.Texture2DArray.ArraySize = 1;
				srv.Texture2DArray.PlaneSlice = 0;
				srv.Texture2DArray.ResourceMinLODClamp = 0.0f;
				break;
			}
			case GFX::Resource::Texture::Type::Tex3D:
			{
				mipLevels = &srv.Texture3D.MipLevels;
				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
				srv.Texture3D.MostDetailedMip = 0;
				srv.Texture3D.ResourceMinLODClamp = 0.0f;
				break;
			}
			case GFX::Resource::Texture::Type::Cube:
			{
				mipLevels = &srv.TextureCube.MipLevels;
				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				srv.TextureCube.MostDetailedMip = 0;
				srv.TextureCube.ResourceMinLODClamp = 0.0f;
				break;
			}
			}

			// Check actual provided surface count for given texture
			const U16 surfaces = Utils::SafeCast<U16>(tex.Surfaces.size());
			if (surfaces)
			{
				// Create texture based on format of first surface (other surfaces are constrained to be of same size as previous ones)
				const GFX::Surface& startSurface = tex.Surfaces.front();
				srv.Format = DX::GetDXFormat(startSurface.GetFormat());
				D3D12_RESOURCE_DESC1 texDesc = device.GetTextureDesc(startSurface.GetWidth(), startSurface.GetHeight(),
					startSurface.GetDepth() > 1 ? startSurface.GetDepth() : (surfaces > 1 ? surfaces : startSurface.GetArraySize()),
					srv.Format, tex.Type);

				ZE_ASSERT(texDesc.DepthOrArraySize == 1 || (tex.Type != GFX::Resource::Texture::Type::Tex2D && tex.Type != GFX::Resource::Texture::Type::Tex1D),
					"Single texture cannot hold multiple surfaces!");
				ZE_ASSERT(texDesc.DepthOrArraySize == 6 || tex.Type != GFX::Resource::Texture::Type::Cube, "Cube texture should contain 6 surfaces!");

				if (tex.Type == GFX::Resource::Texture::Type::Tex1DArray)
					srv.Texture1DArray.ArraySize = texDesc.DepthOrArraySize;
				if (tex.Type == GFX::Resource::Texture::Type::Tex2DArray)
					srv.Texture2DArray.ArraySize = texDesc.DepthOrArraySize;
				texDesc.MipLevels = startSurface.GetMipCount();
				*mipLevels = startSurface.GetMipCount();

				resInfo = device.CreateTexture(texDesc);
				ZE_DX_SET_ID(resInfo.Resource, "Texture_" + std::to_string(i) + "_ID_" + std::to_string(static_cast<U64>(desc.ResourceID)) + (desc.DebugName.size() ? "_" + desc.DebugName : ""));

				const bool copySrc = desc.Options & GFX::Resource::Texture::PackOption::CopySource;
				if (surfaces > 1)
				{
					for (U16 arrayIndex = 0; const auto & surface : tex.Surfaces)
					{
						const U16 index = arrayIndex++;
						diskManager.AddMemoryTextureArrayRequest(resInfo.Resource.Get(), surface.GetMemory(), Utils::SafeCast<U32>(surface.GetMemorySize()),
							index, surface.GetWidth(), surface.GetHeight(), arrayIndex == surfaces, copySrc);
					}
				}
				else
					diskManager.AddMemoryTextureRequest(resInfo.Resource.Get(), startSurface.GetMemory(), Utils::SafeCast<U32>(startSurface.GetMemorySize()), copySrc);
			}
			else
			{
				srv.Format = DX::GetDXFormat(Settings::BackbufferFormat);
				*mipLevels = 1;
			}

			D3D12_CPU_DESCRIPTOR_HANDLE handle = descInfo.CPU;
			handle.ptr += Utils::SafeCast<U64>(i++) * device.GetDescriptorSize();
			ZE_DX_THROW_FAILED_INFO(device.GetDevice()->CreateShaderResourceView(resInfo.Resource.Get(), &srv, handle));
		}
		diskManager.AddTexturePackID(desc.ResourceID);
	}

	Pack::Pack(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::Texture::PackFileDesc& desc, IO::File& file)
	{
		Device& device = dev.Get().dx12;
		DiskManager& diskManager = disk.Get().dx12;
		ZE_DX_ENABLE_ID(device);

		count = Utils::SafeCast<U32>(desc.Textures.size());
		descInfo = device.AllocDescs(count);
		resources = new ResourceInfo[count];

		for (U32 i = 0; const auto & tex : desc.Textures)
		{
			ResourceInfo& resInfo = resources[i];

			// Specify default SRV desc
			D3D12_SHADER_RESOURCE_VIEW_DESC srv = {};
			UINT* mipLevels = nullptr;
			srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			switch (tex.Type)
			{
			case GFX::Resource::Texture::Type::Tex1D:
			{
				mipLevels = &srv.Texture1D.MipLevels;
				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
				srv.Texture1D.MostDetailedMip = 0;
				srv.Texture1D.ResourceMinLODClamp = 0.0f;
				break;
			}
			case GFX::Resource::Texture::Type::Tex1DArray:
			{
				mipLevels = &srv.Texture1DArray.MipLevels;
				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
				srv.Texture1DArray.MostDetailedMip = 0;
				srv.Texture1DArray.FirstArraySlice = 0;
				srv.Texture1DArray.ArraySize = 1;
				srv.Texture1DArray.ResourceMinLODClamp = 0.0f;
				break;
			}
			default:
				ZE_ENUM_UNHANDLED();
			case GFX::Resource::Texture::Type::Tex2D:
			{
				mipLevels = &srv.Texture2D.MipLevels;
				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srv.Texture2D.MostDetailedMip = 0;
				srv.Texture2D.PlaneSlice = 0;
				srv.Texture2D.ResourceMinLODClamp = 0.0f;
				break;
			}
			case GFX::Resource::Texture::Type::Tex2DArray:
			{
				mipLevels = &srv.Texture2DArray.MipLevels;
				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				srv.Texture2DArray.MostDetailedMip = 0;
				srv.Texture2DArray.FirstArraySlice = 0;
				srv.Texture2DArray.ArraySize = 1;
				srv.Texture2DArray.PlaneSlice = 0;
				srv.Texture2DArray.ResourceMinLODClamp = 0.0f;
				break;
			}
			case GFX::Resource::Texture::Type::Tex3D:
			{
				mipLevels = &srv.Texture3D.MipLevels;
				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
				srv.Texture3D.MostDetailedMip = 0;
				srv.Texture3D.ResourceMinLODClamp = 0.0f;
				break;
			}
			case GFX::Resource::Texture::Type::Cube:
			{
				mipLevels = &srv.TextureCube.MipLevels;
				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				srv.TextureCube.MostDetailedMip = 0;
				srv.TextureCube.ResourceMinLODClamp = 0.0f;
				break;
			}
			}

			// Check actual provided surface count for given texture
			if (tex.Format != PixelFormat::Unknown)
			{
				srv.Format = DX::GetDXFormat(tex.Format);
				D3D12_RESOURCE_DESC1 texDesc = device.GetTextureDesc(tex.Width, tex.Height, tex.DepthArraySize, srv.Format, tex.Type);

				ZE_ASSERT(texDesc.DepthOrArraySize == 1 || (tex.Type != GFX::Resource::Texture::Type::Tex2D && tex.Type != GFX::Resource::Texture::Type::Tex1D),
					"Single texture cannot hold multiple surfaces!");
				ZE_ASSERT(texDesc.DepthOrArraySize == 6 || tex.Type != GFX::Resource::Texture::Type::Cube, "Cube texture should contain 6 surfaces!");

				if (tex.Type == GFX::Resource::Texture::Type::Tex1DArray)
					srv.Texture1DArray.ArraySize = texDesc.DepthOrArraySize;
				if (tex.Type == GFX::Resource::Texture::Type::Tex2DArray)
					srv.Texture2DArray.ArraySize = texDesc.DepthOrArraySize;
				texDesc.MipLevels = tex.MipLevels;
				*mipLevels = tex.MipLevels;

				resInfo = device.CreateTexture(texDesc);
				ZE_DX_SET_ID(resInfo.Resource, "Texture_from_file_" + std::to_string(i) + "_" + std::to_string(static_cast<U64>(desc.ResourceID)));

				diskManager.AddFileTextureRequest(resInfo.Resource.Get(), file, tex.DataOffset, tex.SourceBytes, tex.Compression, tex.UncompressedSize, desc.Options & GFX::Resource::Texture::PackOption::CopySource);
			}
			else
			{
				srv.Format = DX::GetDXFormat(Settings::BackbufferFormat);
				*mipLevels = 1;
			}

			D3D12_CPU_DESCRIPTOR_HANDLE handle = descInfo.CPU;
			handle.ptr += Utils::SafeCast<U64>(i++) * device.GetDescriptorSize();
			ZE_DX_THROW_FAILED_INFO(device.GetDevice()->CreateShaderResourceView(resInfo.Resource.Get(), &srv, handle));
		}
		diskManager.AddTexturePackID(desc.ResourceID);
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