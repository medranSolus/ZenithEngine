#include "RHI/DX12/Resource/Generic.h"

namespace ZE::RHI::DX12::Resource
{
	Generic::Generic(GFX::Device& dev, const GFX::Resource::GenericResourceDesc& desc)
	{
		Device& device = dev.Get().dx12;
		ZE_DX_ENABLE_ID(device);

		// Memory info
		D3D12_HEAP_PROPERTIES heapProp = {};
		heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProp.CreationNodeMask = 0;
		heapProp.VisibleNodeMask = 0;

		bool cpuAccess = true;
		D3D12_RESOURCE_STATES initState = GetResourceState(desc.InitState);
		switch (desc.Heap)
		{
		case GFX::Resource::GenericResourceHeap::GPU:
		{
			if (device.IsGpuUploadHeap())
				heapProp.Type = D3D12_HEAP_TYPE_GPU_UPLOAD;
			else
			{
				heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
				cpuAccess = false;
			}
			break;
		}
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::GenericResourceHeap::Upload:
		{
			heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
			initState = D3D12_RESOURCE_STATE_GENERIC_READ;
			break;
		}
		case GFX::Resource::GenericResourceHeap::Readback:
		{
			ZE_ASSERT(desc.InitData == nullptr, "It's not allowed to write to Readback resource!");
			heapProp.Type = D3D12_HEAP_TYPE_READBACK;
			initState = D3D12_RESOURCE_STATE_COPY_DEST;
			break;
		}
		}

		// Parse resource desc
		D3D12_RESOURCE_DESC1 resDesc = {};
		resDesc.Alignment = 0;
		resDesc.Width = desc.WidthOrBufferSize;
		resDesc.Height = desc.HeightOrBufferStride;
		resDesc.DepthOrArraySize = desc.DepthOrArraySize;
		resDesc.MipLevels = desc.MipCount;
		resDesc.Format = DX::GetDXFormat(desc.Format);
		resDesc.SampleDesc.Count = 1;
		resDesc.SampleDesc.Quality = 0;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resDesc.Flags = GetGenericResourceFlags(desc.Flags);
		resDesc.SamplerFeedbackMipRegion.Width = 0;
		resDesc.SamplerFeedbackMipRegion.Height = 0;
		resDesc.SamplerFeedbackMipRegion.Depth = 0;

		switch (desc.Type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::GenericResourceType::Buffer:
		{
			resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			resDesc.Height = 1;
			resDesc.DepthOrArraySize = 1;
			resDesc.MipLevels = 1;
			resDesc.Format = DXGI_FORMAT_UNKNOWN;
			resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			break;
		}
		case GFX::Resource::GenericResourceType::Texture1D:
		{
			resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
			resDesc.Height = 1;
			break;
		}
		case GFX::Resource::GenericResourceType::Texture2D:
		{
			resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			break;
		}
		case GFX::Resource::GenericResourceType::TextureCube:
		case GFX::Resource::GenericResourceType::Texture3D:
		{
			resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
			break;
		}
		}

		// Clear value not used by buffers
		D3D12_CLEAR_VALUE clearVal = {};
		clearVal.Format = resDesc.Format;
		if (resDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
		{
			clearVal.DepthStencil.Depth = 0.0f;
			clearVal.DepthStencil.Stencil = 0;
		}
		else
			clearVal.Color[0] = clearVal.Color[1] = clearVal.Color[2] = clearVal.Color[3] = 0.0f;

		ZE_DX_THROW_FAILED(device.GetDevice()->CreateCommittedResource2(&heapProp,
			D3D12_HEAP_FLAG_CREATE_NOT_ZEROED, &resDesc, initState,
			resDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER
			|| (resDesc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)) == 0 ? nullptr : &clearVal,
			nullptr, IID_PPV_ARGS(&resource)));
		ZE_DX_SET_ID(resource, desc.DebugName.c_str());

		if (cpuAccess)
		{
			const D3D12_RANGE range = {};
			ZE_DX_THROW_FAILED(resource->Map(0, &range, reinterpret_cast<void**>(&buffer)));

			if (desc.InitData && !IsStagingCopyRequired(dev, desc))
			{
				// Get upload info
				D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
				UINT rowCount = 0;
				UINT64 rowSizeInBytes = 0;
				UINT64 totalBytes = 0;
				device.GetDevice()->GetCopyableFootprints1(&resDesc, 0, 1, 0, &footprint, &rowCount, &rowSizeInBytes, &totalBytes);

				U8* src = reinterpret_cast<U8*>(desc.InitData);
				U8* dest = buffer;
				for (U32 currentRowIndex = 0; currentRowIndex < resDesc.Height; ++currentRowIndex)
				{
					std::memcpy(dest, src, rowSizeInBytes);
					src += rowSizeInBytes;
					dest += footprint.Footprint.RowPitch;
				}
			}
		}

		// Create views
		const bool arrayView = (desc.Flags & GFX::Resource::GenericResourceFlag::ArrayView) || resDesc.DepthOrArraySize > 1;
		D3D12_UNORDERED_ACCESS_VIEW_DESC uav = {};
		D3D12_SHADER_RESOURCE_VIEW_DESC srv = {};
		srv.Format = uav.Format = resDesc.Format;
		srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		switch (desc.Type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::GenericResourceType::Buffer:
		{
			uav.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uav.Buffer.FirstElement = 0;
			uav.Buffer.NumElements = desc.WidthOrBufferSize;
			// View is not structured view so format is required
			if (uav.Buffer.StructureByteStride == 0)
			{
				uav.Format = DX::GetDXFormat(desc.Format);
			}
			else
				uav.Buffer.NumElements /= desc.HeightOrBufferStride;
			uav.Buffer.StructureByteStride = desc.HeightOrBufferStride;
			uav.Buffer.CounterOffsetInBytes = 0;
			uav.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

			srv.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srv.Buffer.FirstElement = 0;
			srv.Buffer.NumElements = uav.Buffer.NumElements;
			srv.Buffer.StructureByteStride = uav.Buffer.StructureByteStride;
			srv.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			break;
		}
		case GFX::Resource::GenericResourceType::Texture1D:
		{
			if (arrayView)
			{
				uav.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
				uav.Texture1DArray.MipSlice = 0;
				uav.Texture1DArray.FirstArraySlice = 0;
				uav.Texture1DArray.ArraySize = resDesc.DepthOrArraySize;

				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
				srv.Texture1DArray.MostDetailedMip = 0;
				srv.Texture1DArray.MipLevels = resDesc.MipLevels;
				srv.Texture1DArray.FirstArraySlice = 0;
				srv.Texture1DArray.ArraySize = resDesc.DepthOrArraySize;
				srv.Texture1DArray.ResourceMinLODClamp = 0.0f;
			}
			else
			{
				uav.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
				uav.Texture1D.MipSlice = 0;

				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
				srv.Texture1D.MostDetailedMip = 0;
				srv.Texture1D.MipLevels = resDesc.MipLevels;
				srv.Texture1D.ResourceMinLODClamp = 0.0f;
			}
			break;
		}
		case GFX::Resource::GenericResourceType::Texture2D:
		{
			if (arrayView)
			{
				uav.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
				uav.Texture2DArray.MipSlice = 0;
				uav.Texture2DArray.FirstArraySlice = 0;
				uav.Texture2DArray.ArraySize = resDesc.DepthOrArraySize;
				uav.Texture2DArray.PlaneSlice = 0;

				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				srv.Texture2DArray.MostDetailedMip = 0;
				srv.Texture2DArray.MipLevels = resDesc.MipLevels;
				srv.Texture2DArray.FirstArraySlice = 0;
				srv.Texture2DArray.ArraySize = resDesc.DepthOrArraySize;
				srv.Texture2DArray.PlaneSlice = 0;
				srv.Texture2DArray.ResourceMinLODClamp = 0.0f;
			}
			else
			{
				uav.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uav.Texture2D.MipSlice = 0;
				uav.Texture2D.PlaneSlice = 0;

				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srv.Texture2D.MostDetailedMip = 0;
				srv.Texture2D.MipLevels = resDesc.MipLevels;
				srv.Texture2D.PlaneSlice = 0;
				srv.Texture2D.ResourceMinLODClamp = 0.0f;
			}
			break;
		}
		case GFX::Resource::GenericResourceType::TextureCube:
		{
			uav.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
			uav.Texture3D.MipSlice = 0;
			uav.Texture3D.FirstWSlice = 0;
			uav.Texture3D.WSize = UINT32_MAX;

			if (arrayView)
			{
				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
				srv.TextureCubeArray.MostDetailedMip = 0;
				srv.TextureCubeArray.MipLevels = resDesc.MipLevels;
				srv.TextureCubeArray.First2DArrayFace = 0;
				srv.TextureCubeArray.NumCubes = resDesc.DepthOrArraySize;
				srv.TextureCubeArray.ResourceMinLODClamp = 0.0f;
			}
			else
			{
				srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				srv.TextureCube.MostDetailedMip = 0;
				srv.TextureCube.MipLevels = resDesc.MipLevels;
				srv.TextureCube.ResourceMinLODClamp = 0.0f;
			}
			break;
		}
		case GFX::Resource::GenericResourceType::Texture3D:
		{
			uav.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
			uav.Texture3D.MipSlice = 0;
			uav.Texture3D.FirstWSlice = 0;
			uav.Texture3D.WSize = UINT32_MAX;

			srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			srv.Texture3D.MostDetailedMip = 0;
			srv.Texture3D.MipLevels = resDesc.MipLevels;
			srv.Texture3D.ResourceMinLODClamp = 0.0f;
			break;
		}
		}

		if (desc.Heap != GFX::Resource::GenericResourceHeap::Readback)
		{
			// SRV
			srvDescriptor = device.AllocDescs(1);
			device.GetDevice()->CreateShaderResourceView(resource.Get(), &srv, srvDescriptor.CPU);

			// UAV
			if (desc.Flags & GFX::Resource::GenericResourceFlag::UnorderedAccess)
			{
				uavDescriptorCpu = device.AllocDescs(resDesc.MipLevels, false);
				device.GetDevice()->CreateUnorderedAccessView(resource.Get(), nullptr, &uav, uavDescriptorCpu.CPU);

				if (desc.Type != GFX::Resource::GenericResourceType::Buffer)
				{
					UINT* destMip = nullptr;
					switch (uav.ViewDimension)
					{
					default:
					{
						ZE_FAIL("Incorrect dimmension for UAV view!");
						resDesc.MipLevels = 0;
						break;
					}
					case D3D12_UAV_DIMENSION_TEXTURE1D:
						destMip = &uav.Texture1D.MipSlice;
						break;
					case D3D12_UAV_DIMENSION_TEXTURE1DARRAY:
						destMip = &uav.Texture1DArray.MipSlice;
						break;
					case D3D12_UAV_DIMENSION_TEXTURE2D:
						destMip = &uav.Texture2D.MipSlice;
						break;
					case D3D12_UAV_DIMENSION_TEXTURE2DARRAY:
						destMip = &uav.Texture2DArray.MipSlice;
						break;
					case D3D12_UAV_DIMENSION_TEXTURE3D:
						destMip = &uav.Texture3D.MipSlice;
						break;
					}

					D3D12_CPU_DESCRIPTOR_HANDLE descHandle = uavDescriptorCpu.CPU;
					for (U16 mip = 1; mip < resDesc.MipLevels; ++mip)
					{
						descHandle.ptr += device.GetDescriptorSize();
						*destMip = mip;
						device.GetDevice()->CreateUnorderedAccessView(resource.Get(), nullptr, &uav, descHandle);
					}
				}

				uavDescriptorGpu = device.AllocDescs(resDesc.MipLevels);
				device.GetDevice()->CopyDescriptorsSimple(resDesc.MipLevels, uavDescriptorGpu.CPU, uavDescriptorCpu.CPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}
		}
	}

	bool Generic::IsStagingCopyRequired(GFX::Device& dev, const GFX::Resource::GenericResourceDesc& desc) const noexcept
	{
		if (desc.InitData)
		{
			ZE_ASSERT(desc.InitDataSize, "Data size must be non-zero!");

			if (desc.Heap == GFX::Resource::GenericResourceHeap::GPU)
			{
				if (!dev.Get().dx12.IsGpuUploadHeap())
					return true;
				return desc.Type != GFX::Resource::GenericResourceType::Buffer;
			}
		}
		return false;
	}

	void Generic::Free(GFX::Device& dev) noexcept
	{
		resource = nullptr;
		buffer = nullptr;
		if (srvDescriptor.Handle)
			dev.Get().dx12.FreeDescs(srvDescriptor);
		if (uavDescriptorCpu.Handle)
			dev.Get().dx12.FreeDescs(uavDescriptorCpu);
		if (uavDescriptorGpu.Handle)
			dev.Get().dx12.FreeDescs(uavDescriptorGpu);
	}
}