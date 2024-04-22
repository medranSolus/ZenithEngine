#include "RHI/DX12/Pipeline/FrameBuffer.h"
#include "Data/Camera.h"
#include "GFX/XeSSException.h"

namespace ZE::RHI::DX12::Pipeline
{
#if !_ZE_MODE_RELEASE
	void FrameBuffer::PrintMemory(std::string&& memID, U32 levelCount, U64 heapSize,
		std::vector<ResourcInitInfo>::iterator resBegin, std::vector<ResourcInitInfo>::iterator resEnd,
		const std::vector<std::pair<U32, U32>>& resourcesLifetime) noexcept
	{
		U32 maxChunks = 0;
		for (auto it = resBegin; it != resEnd; ++it)
			maxChunks += it->Chunks;

		const U32 pixelsPerLevel = maxChunks / levelCount;
		U32 separatorPixels = pixelsPerLevel / 20;
		if (separatorPixels < 2)
			separatorPixels = 2;
		const U32 chunkPixels = pixelsPerLevel - separatorPixels;

		GFX::Surface print(levelCount * pixelsPerLevel, maxChunks, PixelFormat::R8G8B8A8_UNorm);
		Pixel* image = reinterpret_cast<Pixel*>(print.GetBuffer());
		// Clear output image
		for (U32 y = 0; y < print.GetHeight(); ++y)
			for (U32 x = 0; x < print.GetWidth(); ++x)
				image[y * print.GetWidth() + x] = PixelVal::Black;

		// Write regions for all resources
		for (; resBegin != resEnd; ++resBegin)
		{
			// Compute resource color
			const U64 val = resBegin->Handle / resourcesLifetime.size();
			const Pixel pixel(static_cast<U8>(val >> (8 * (val % 3))),
				static_cast<U8>(val >> (8 * ((val + 1) % 3))),
				static_cast<U8>(val >> (8 * ((val + 2) % 3))),
				static_cast<U8>(val >> 24) ^ 0xFF);

			// Fill resorce rectangle
			for (U32 chunk = 0; chunk < resBegin->Chunks; ++chunk)
			{
				resBegin->ChunkOffset;
				U32 startLevel = resourcesLifetime.at(resBegin->Handle).first;
				U32 levelCount = resourcesLifetime.at(resBegin->Handle).second;
				for (U32 level = 0; level < levelCount; ++level)
				{
					const U32 offset = chunk * maxChunks + level * pixelsPerLevel;
					for (U64 p = 0; p < chunkPixels; ++p)
						image[offset + p] = pixel;
				}
			}
		}

		// Create separators for every level
		for (U32 chunk = 0; chunk < maxChunks; ++chunk)
		{
			for (U32 level = 0; level < levelCount; ++level)
			{
				const U32 offset = chunk * maxChunks + level * pixelsPerLevel + chunkPixels;
				for (U64 p = 0; p < separatorPixels; ++p)
					image[offset + p] = PixelVal::White;
			}
		}
		print.Save("memory_print_dx12_" + memID + "_" + std::to_string(heapSize) + "B.png");
	}
#endif

	U64 FrameBuffer::AllocateResources(std::vector<ResourcInitInfo>::iterator resBegin, std::vector<ResourcInitInfo>::iterator resEnd,
		const std::vector<std::pair<U32, U32>>& resourcesLifetime, U32 levelCount, GFX::Pipeline::FrameBufferFlags flags) noexcept
	{
		U32 heapChunks = 0;

		// Other algorithm: https://stackoverflow.com/questions/25683078/algorithm-for-packing-time-slots
		if (flags & GFX::Pipeline::FrameBufferFlag::NoMemoryAliasing)
		{
			// No resource aliasing so place all of the one after another
			for (; resBegin != resEnd; ++resBegin)
			{
				resBegin->ChunkOffset = heapChunks;
				heapChunks += resBegin->Chunks;
			}
		}
		else
		{
			// Find max size of the heap
			U32 maxChunks = 0;
			for (auto it = resBegin; it != resEnd; ++it)
				maxChunks += it->Chunks;

			// Find free memory regions for resources
			std::vector<RID> memory(maxChunks * levelCount, INVALID_RID);
			for (auto it = resBegin; it != resEnd; ++it)
			{
				U32 startLevel = 0, endLevel = levelCount;
				if (!it->IsTemporal())
				{
					startLevel = resourcesLifetime.at(it->Handle).first;
					endLevel = startLevel + resourcesLifetime.at(it->Handle).second;
				}

				U32 foundOffset = 0;
				// Search through whole memory
				for (U32 offset = 0, chunksFound = 0; offset < maxChunks; ++offset)
				{
					// Check chunks for whole requested duration
					for (U32 time = startLevel; time < endLevel; ++time)
					{
						if (memory.at(offset * levelCount + time) != INVALID_RID)
						{
							foundOffset = maxChunks;
							break;
						}
					}
					if (foundOffset != maxChunks)
					{
						if (++chunksFound == it->Chunks)
							break;
					}
					else
					{
						chunksFound = 0;
						foundOffset = offset + 1;
					}
				}
				ZE_ASSERT(foundOffset + it->Chunks <= maxChunks, "Memory too small to fit requested resource, bug in memory table creation!");

				// Reserve space in memory
				for (U32 chunk = 0; chunk < it->Chunks; ++chunk)
					std::fill_n(memory.begin() + Utils::SafeCast<U64>(foundOffset + chunk) * levelCount + startLevel, endLevel - startLevel, it->Handle);
				it->ChunkOffset = foundOffset;
			}

			// Find last chunk of the heap and remove and use it as heap size
			U32 lastChunk = 0;
			for (U32 chunk = 0; chunk < maxChunks; ++chunk)
			{
				bool notFoundInLevel = true;
				for (U32 level = 0; level < levelCount; ++level)
				{
					if (memory.at(chunk * levelCount + level) != INVALID_RID)
					{
						heapChunks = chunk;
						notFoundInLevel = false;
						break;
					}
				}
				if (notFoundInLevel)
					break;
			}
			++heapChunks;
		}
		return Utils::SafeCast<U64>(heapChunks) * D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
	}

	void FrameBuffer::EnterRaster() const noexcept
	{
#if !_ZE_MODE_RELEASE
		ZE_ASSERT(!isRasterActive, "Starting rasterization without calling EndRaster()!");

		isRasterActive = true;
#endif
	}

	void FrameBuffer::SetupViewport(D3D12_VIEWPORT& viewport, D3D12_RECT& scissorRect, RID rid) const noexcept
	{
		scissorRect.left = 0;
		scissorRect.top = 0;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;

		const UInt2 size = resources[rid].Size;
		scissorRect.right = size.X;
		scissorRect.bottom = size.Y;
		viewport.Width = Utils::SafeCast<float>(scissorRect.right);
		viewport.Height = Utils::SafeCast<float>(scissorRect.bottom);

		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
	}

	void FrameBuffer::SetViewport(CommandList& cl, RID rid) const noexcept
	{
		D3D12_VIEWPORT viewport = {};
		D3D12_RECT scissorRect = {};
		SetupViewport(viewport, scissorRect, rid);
		cl.GetList()->RSSetViewports(1, &viewport);
		cl.GetList()->RSSetScissorRects(1, &scissorRect);
	}

	void FrameBuffer::FillBarier(D3D12_TEXTURE_BARRIER& barrier, const GFX::Pipeline::BarrierTransition& desc) const noexcept
	{
		ZE_ASSERT(desc.Resource < resourceCount, "Resource ID outside available range!");

		ZE_ASSERT(desc.LayoutAfter != GFX::Pipeline::TextureLayout::Undefined && desc.LayoutAfter != GFX::Pipeline::TextureLayout::Preinitialized,
			"Undefined and preinitialized layouts are only possible for resources written by CPU!");

		ZE_ASSERT(desc.LayoutBefore != GFX::Pipeline::TextureLayout::DepthStencilRead ||
			desc.Resource != BACKBUFFER_RID && desc.LayoutBefore == GFX::Pipeline::TextureLayout::DepthStencilRead && rtvDsvHandles[desc.Resource].ptr != UINT64_MAX,
			"Current resource is not suitable for being depth stencil!");
		ZE_ASSERT(desc.LayoutAfter != GFX::Pipeline::TextureLayout::DepthStencilRead ||
			desc.Resource != BACKBUFFER_RID && desc.LayoutAfter == GFX::Pipeline::TextureLayout::DepthStencilRead && rtvDsvHandles[desc.Resource].ptr != UINT64_MAX,
			"Current resource is not suitable for being depth stencil!");
		ZE_ASSERT(desc.LayoutBefore != GFX::Pipeline::TextureLayout::DepthStencilWrite ||
			desc.Resource != BACKBUFFER_RID && desc.LayoutBefore == GFX::Pipeline::TextureLayout::DepthStencilRead && rtvDsvHandles[desc.Resource].ptr != UINT64_MAX,
			"Current resource is not suitable for being depth stencil!");
		ZE_ASSERT(desc.LayoutAfter != GFX::Pipeline::TextureLayout::DepthStencilWrite ||
			desc.Resource != BACKBUFFER_RID && desc.LayoutAfter == GFX::Pipeline::TextureLayout::DepthStencilRead && rtvDsvHandles[desc.Resource].ptr != UINT64_MAX,
			"Current resource is not suitable for being depth stencil!");

		ZE_ASSERT(desc.LayoutBefore != GFX::Pipeline::TextureLayout::RenderTarget ||
			desc.LayoutBefore == GFX::Pipeline::TextureLayout::RenderTarget && rtvDsvHandles[desc.Resource].ptr != UINT64_MAX,
			"Current resource is not suitable for being render target!");
		ZE_ASSERT(desc.LayoutAfter != GFX::Pipeline::TextureLayout::RenderTarget ||
			desc.LayoutAfter == GFX::Pipeline::TextureLayout::RenderTarget && rtvDsvHandles[desc.Resource].ptr != UINT64_MAX,
			"Current resource is not suitable for being render target!");

		ZE_ASSERT(desc.LayoutBefore != GFX::Pipeline::TextureLayout::ShaderResource ||
			desc.LayoutBefore == GFX::Pipeline::TextureLayout::ShaderResource && GetSRV(desc.Resource).CpuShaderVisibleHandle.ptr != UINT64_MAX,
			"Current resource is not suitable for being shader resource!");
		ZE_ASSERT(desc.LayoutAfter != GFX::Pipeline::TextureLayout::ShaderResource ||
			desc.LayoutAfter == GFX::Pipeline::TextureLayout::ShaderResource && GetSRV(desc.Resource).CpuShaderVisibleHandle.ptr != UINT64_MAX,
			"Current resource is not suitable for being shader resource!");

		ZE_ASSERT(desc.LayoutBefore != GFX::Pipeline::TextureLayout::UnorderedAccess ||
			desc.LayoutBefore == GFX::Pipeline::TextureLayout::UnorderedAccess && GetUAV(desc.Resource).CpuHandle.ptr != UINT64_MAX,
			"Current resource is not suitable for being unnordered access!");
		ZE_ASSERT(desc.LayoutAfter != GFX::Pipeline::TextureLayout::UnorderedAccess ||
			desc.LayoutAfter == GFX::Pipeline::TextureLayout::UnorderedAccess && GetUAV(desc.Resource).CpuHandle.ptr != UINT64_MAX,
			"Current resource is not suitable for being unnordered access!");

		barrier.SyncBefore = GetBarrierSync(desc.StageBefore);
		barrier.SyncAfter = GetBarrierSync(desc.StageAfter);
		switch (desc.Type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Pipeline::BarrierType::Immediate:
			break;
		case GFX::Pipeline::BarrierType::SplitBegin:
			barrier.SyncAfter |= D3D12_BARRIER_SYNC_SPLIT;
			break;
		case GFX::Pipeline::BarrierType::SplitEnd:
			barrier.SyncBefore |= D3D12_BARRIER_SYNC_SPLIT;
			break;
		}
		barrier.AccessBefore = GetBarrierAccess(desc.AccessBefore);
		barrier.AccessAfter = GetBarrierAccess(desc.AccessAfter);
		barrier.LayoutBefore = GetBarrierLayout(desc.LayoutBefore);
		barrier.LayoutAfter = GetBarrierLayout(desc.LayoutAfter);
		barrier.pResource = GetResource(desc.Resource).Get();
		barrier.Subresources.IndexOrFirstMipLevel = UINT32_MAX;
		barrier.Subresources.NumMipLevels = 0;
		barrier.Subresources.FirstArraySlice = 0;
		barrier.Subresources.NumArraySlices = 0;
		barrier.Subresources.FirstPlane = 0;
		barrier.Subresources.NumPlanes = 0;
		barrier.Flags = desc.LayoutBefore == GFX::Pipeline::TextureLayout::Undefined ? D3D12_TEXTURE_BARRIER_FLAG_DISCARD : D3D12_TEXTURE_BARRIER_FLAG_NONE;
	}

	void FrameBuffer::PerformBarrier(CommandList& cl, const D3D12_TEXTURE_BARRIER* barriers, U32 count) const noexcept
	{
		D3D12_BARRIER_GROUP group;
		group.Type = D3D12_BARRIER_TYPE_TEXTURE;
		group.NumBarriers = count;
		group.pTextureBarriers = barriers;
		cl.GetList()->Barrier(1, &group);
	}

	FrameBuffer::FrameBuffer(GFX::Device& dev, const GFX::Pipeline::FrameBufferDesc& desc)
	{
		ZE_ASSERT(desc.Resources.size() > 0, "Empty FrameBuffer!");
		ZE_ASSERT(desc.Resources.size() == desc.ResourceLifetimes.size(), "Not every resource have it's associated lifetime!");
		ZE_ASSERT(desc.PassLevelCount > 0, "At least single pass level is required for passes to execute!");

		ZE_DX_ENABLE_ID(dev.Get().dx12);
		IDevice* device = dev.Get().dx12.GetDevice();

		resourceCount = Utils::SafeCast<RID>(desc.Resources.size());
		RID rtvCount = 0, rtvAdditionalMipsCount = 0;
		RID dsvCount = 0, dsvAdditionalMipsCount = 0;
		RID srvCount = 0, srvUavCount = 0;
		RID uavCount = 0, uavAdditionalMipsCount = 0;

		// Get sizes in chunks for resources and their descriptors
		std::vector<ResourcInitInfo> resourcesInfo;
		D3D12_RESOURCE_DESC1 resDesc = {};
		resDesc.SampleDesc.Count = 1;
		resDesc.SampleDesc.Quality = 0;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resDesc.SamplerFeedbackMipRegion.Width = 0;
		resDesc.SamplerFeedbackMipRegion.Height = 0;
		resDesc.SamplerFeedbackMipRegion.Depth = 0;

		for (RID i = 1; i < resourceCount; ++i)
		{
			const auto& res = desc.Resources.at(i);
			if (res.Flags & GFX::Pipeline::FrameResourceFlag::InternalResourceActive)
			{
				if (res.Flags & GFX::Pipeline::FrameResourceFlag::Texture3D)
					resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
				else
					resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				resDesc.Width = res.Sizes.X;
				resDesc.Height = res.Sizes.Y;
				resDesc.DepthOrArraySize = res.DepthOrArraySize;
				if (res.Flags & GFX::Pipeline::FrameResourceFlag::Cube)
				{
					ZE_ASSERT(resDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D, "Cannot create cubemap texture as 3D texture!");

					resDesc.DepthOrArraySize *= 6;
				}
				resDesc.MipLevels = res.MipLevels;
				if (!resDesc.MipLevels)
					resDesc.MipLevels = 1;
				resDesc.Format = DX::GetTypedDepthDXFormat(res.Format);
				if (res.Flags & GFX::Pipeline::FrameResourceFlag::SimultaneousAccess)
					resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
				else
					resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

				D3D12_CLEAR_VALUE clearDesc = {};
				clearDesc.Format = resDesc.Format;
				// Check usage flags
				bool isRT = res.Flags & GFX::Pipeline::FrameResourceFlag::InternalUsageRenderTarget;
				bool isDS = res.Flags & (GFX::Pipeline::FrameResourceFlag::ForceDSV | GFX::Pipeline::FrameResourceFlag::InternalUsageDepth);
				bool isUA = res.Flags & GFX::Pipeline::FrameResourceFlag::InternalUsageUnorderedAccess;
				bool isSR = res.Flags & (GFX::Pipeline::FrameResourceFlag::ForceSRV | GFX::Pipeline::FrameResourceFlag::InternalUsageShaderResource);
				if (isRT)
				{
					ZE_ASSERT(!isDS, "Cannot create depth stencil and render target view for same buffer!");
					ZE_ASSERT(!Utils::IsDepthStencilFormat(res.Format), "Cannot use depth stencil format with render target!");

					resDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
					++rtvCount;
					if (resDesc.MipLevels > 1)
						rtvAdditionalMipsCount += resDesc.MipLevels - 1;
				}
				if (isDS)
				{
					ZE_ASSERT(resDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D, "Cannot create 3D texture as depth stencil!");
					ZE_ASSERT((res.Flags & GFX::Pipeline::FrameResourceFlag::SimultaneousAccess) == 0, "Simultaneous access cannot be used on depth stencil!");
					ZE_ASSERT(!isRT, "Cannot create depth stencil and render target view for same buffer!");
					ZE_ASSERT(!isUA, "Cannot create depth stencil and unordered access view for same buffer!");

					resDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
					clearDesc.DepthStencil.Depth = res.ClearDepth;
					clearDesc.DepthStencil.Stencil = res.ClearStencil;
					++dsvCount;
					if (resDesc.MipLevels > 1)
						dsvAdditionalMipsCount += resDesc.MipLevels - 1;
				}
				else
					*reinterpret_cast<ColorF4*>(clearDesc.Color) = res.ClearColor;
				if (isUA)
				{
					ZE_ASSERT(!isDS, "Cannot create depth stencil and unordered access view for same buffer!");

					resDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
					++uavCount;
					if (resDesc.MipLevels > 1)
						uavAdditionalMipsCount += resDesc.MipLevels - 1;
				}
				if (isSR)
					++srvCount;
				else
					resDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
				if (isSR || isUA)
					++srvUavCount;

				// Get resource alignment and size in chunks
				resDesc.Alignment = D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
				D3D12_RESOURCE_ALLOCATION_INFO1 allocInfo = {};
				device->GetResourceAllocationInfo2(0, 1, &resDesc, &allocInfo);
				if (allocInfo.Alignment != D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT)
					resDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

				// Create resource entry and fill it with proper info
				const U64 chunksCount = Math::DivideRoundUp(allocInfo.SizeInBytes, static_cast<U64>(D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT));
				auto& info = resourcesInfo.emplace_back(i, Utils::SafeCast<U32>(chunksCount), 0, resDesc, clearDesc, res.Flags, 0);
				if (res.Flags & GFX::Pipeline::FrameResourceFlag::Cube)
					info.SetCube();
				if (res.Flags & GFX::Pipeline::FrameResourceFlag::StencilView)
					info.SetStencilView();
				if (res.Flags & GFX::Pipeline::FrameResourceFlag::Temporal)
					info.SetTemporal();
			}
		}
		ZE_ASSERT(resourcesInfo.size() > 0, "No active resource in the frame!");

		// Prepare data for allocating resources and heaps
		const D3D12_RESIDENCY_PRIORITY residencyPriority = D3D12_RESIDENCY_PRIORITY_MAXIMUM;
		D3D12_HEAP_DESC heapDesc = {};
		heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapDesc.Properties.CreationNodeMask = 0;
		heapDesc.Properties.VisibleNodeMask = 0;
		heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		heapDesc.Flags = D3D12_HEAP_FLAG_CREATE_NOT_ZEROED;

		RID mainHeapResourceCount;
		// Handle resource types (non RT/DS) depending on present tier level
		if (dev.Get().dx12.GetCurrentAllocTier() == AllocatorGPU::AllocTier::Tier1)
		{
			heapDesc.Flags |= D3D12_HEAP_FLAG_DENY_BUFFERS;
			mainHeapResourceCount = rtvCount + dsvCount;
			// Sort resources descending by size leaving UAV only on the end
			std::sort(resourcesInfo.begin(), resourcesInfo.end(),
				[](const auto& r1, const auto& r2) -> bool
				{
					const bool r1RTV_DSV = r1.Desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
					const bool r2RTV_DSV = r1.Desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
					if (r1RTV_DSV == r2RTV_DSV)
						return r1.Chunks > r2.Chunks;
					return r1RTV_DSV;
				});

			// Create heap for non RT or DS buffers
			if (mainHeapResourceCount < Utils::SafeCast<RID>(resourcesInfo.size()))
			{
				// Find offsets for all resources in this heap and get it's size
				heapDesc.SizeInBytes = AllocateResources(resourcesInfo.begin() + mainHeapResourceCount, resourcesInfo.end(), desc.ResourceLifetimes, desc.PassLevelCount, desc.Flags);

				heapDesc.Flags |= D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES;
				ZE_DX_THROW_FAILED(device->CreateHeap1(&heapDesc, nullptr, IID_PPV_ARGS(&uavHeap)));
				ZE_DX_SET_ID(uavHeap, "GFX::Pipeline::FrameBuffer heap - UAV");
				ZE_DX_THROW_FAILED(device->SetResidencyPriority(1, reinterpret_cast<IPageable**>(uavHeap.GetAddressOf()), &residencyPriority));
				heapDesc.Flags &= ~D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES;
				heapDesc.Flags |= D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;

#if !_ZE_MODE_RELEASE
				if (desc.Flags & GFX::Pipeline::FrameBufferFlag::DebugMemoryPrint)
					PrintMemory("tier1_uav", desc.PassLevelCount, heapDesc.SizeInBytes,
						resourcesInfo.begin() + mainHeapResourceCount, resourcesInfo.end(), desc.ResourceLifetimes);
#endif
				// Set all resources as using UAV heap for creation later
				for (auto it = resourcesInfo.begin() + mainHeapResourceCount; it != resourcesInfo.end(); ++it)
					it->SetHeapUAV();
			}
		}
		else
		{
			mainHeapResourceCount = Utils::SafeCast<RID>(resourcesInfo.size());
			// Sort resources descending by size
			std::sort(resourcesInfo.begin(), resourcesInfo.end(),
				[](const auto& r1, const auto& r2) -> bool
				{
					return r1.Chunks > r2.Chunks;
				});
		}

		// Allocate resources and create main heap
		heapDesc.SizeInBytes = AllocateResources(resourcesInfo.begin(), resourcesInfo.begin() + mainHeapResourceCount, desc.ResourceLifetimes, desc.PassLevelCount, desc.Flags);
		ZE_DX_THROW_FAILED(device->CreateHeap1(&heapDesc, nullptr, IID_PPV_ARGS(&mainHeap)));
		ZE_DX_SET_ID(uavHeap, "GFX::Pipeline::FrameBuffer heap - main");
		ZE_DX_THROW_FAILED(device->SetResidencyPriority(1, reinterpret_cast<IPageable**>(mainHeap.GetAddressOf()), &residencyPriority));

#if !_ZE_MODE_RELEASE
		if (desc.Flags & GFX::Pipeline::FrameBufferFlag::DebugMemoryPrint)
			PrintMemory(dev.Get().dx12.GetCurrentAllocTier() == AllocatorGPU::AllocTier::Tier1 ? "tier1" : "tier2",
				desc.PassLevelCount, heapDesc.SizeInBytes, resourcesInfo.begin(), resourcesInfo.begin() + mainHeapResourceCount, desc.ResourceLifetimes);
#endif

		// Sort resources for increasing RID to properly create adjacent descriptors later on
		std::sort(resourcesInfo.begin(), resourcesInfo.end(),
			[](const auto& r1, const auto& r2) -> bool
			{
				return r1.Handle < r2.Handle;
			});

		// Create all resources and fill their info
		resources = new BufferData[resourceCount];
		resources[BACKBUFFER_RID].Resource = nullptr;
		resources[BACKBUFFER_RID].Size = desc.Resources.front().Sizes;
		resources[BACKBUFFER_RID].Array = desc.Resources.front().DepthOrArraySize;
		resources[BACKBUFFER_RID].Mips = desc.Resources.front().MipLevels;
		resources[BACKBUFFER_RID].Format = desc.Resources.front().Format;
		for (auto& res : resourcesInfo)
		{
			auto& data = resources[res.Handle];
			ZE_DX_THROW_FAILED(device->CreatePlacedResource2(res.IsHeapUAV() ? uavHeap.Get() : mainHeap.Get(),
				res.ChunkOffset * D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT, &res.Desc, D3D12_BARRIER_LAYOUT_UNDEFINED,
				res.Desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) ? &res.ClearVal : nullptr,
				0, nullptr, IID_PPV_ARGS(&data.Resource)));
			ZE_DX_SET_ID(data.Resource, "RID_" + std::to_string(res.Handle) + (desc.Resources.at(res.Handle).DebugName.size() ? " " + desc.Resources.at(res.Handle).DebugName : ""));

			data.Size = { Utils::SafeCast<U32>(res.Desc.Width), res.Desc.Height };
			data.Array = res.Desc.DepthOrArraySize;
			data.Mips = res.Desc.MipLevels;
			data.Format = DX::GetFormatFromDX(res.Desc.Format);
			if (res.Desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
				data.SetTexture3D();
			else
			{
				if (res.Desc.DepthOrArraySize > 1)
					data.SetArrayView();
				if (res.IsCube())
					data.SetCube();
			}
		}

		// Create descriptor heaps
		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
		descHeapDesc.NodeMask = 0;
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descHeapDesc.NumDescriptors = rtvCount + rtvAdditionalMipsCount;
		ZE_DX_THROW_FAILED(device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&rtvDescHeap)));
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		descHeapDesc.NumDescriptors = dsvCount + dsvAdditionalMipsCount;
		ZE_DX_THROW_FAILED(device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&dsvDescHeap)));

		// Prepare descriptors creation
		rtvDsvHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[resourceCount];
		srvHandles = new HandleSRV[resourceCount];
		uavHandles = new HandleUAV[resourceCount - 1];
		if (rtvAdditionalMipsCount + dsvAdditionalMipsCount)
			rtvDsvMips = new Ptr<D3D12_CPU_DESCRIPTOR_HANDLE>[resourceCount - 1];
		if (uavAdditionalMipsCount)
			uavMips = new Ptr<HandleUAV>[resourceCount - 1];

		const U32 rtvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		const U32 dsvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		const U32 srvUavDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescHeap->GetCPUDescriptorHandleForHeapStart();
		const RID uavDescCount = uavCount + uavAdditionalMipsCount;
		descInfo = dev.Get().dx12.AllocDescs(srvCount + uavDescCount);
		if (uavDescCount)
			descInfoCpu = dev.Get().dx12.AllocDescs(uavDescCount, false);

		D3D12_CPU_DESCRIPTOR_HANDLE srvUavShaderVisibleHandle = descInfo.CPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvUavShaderVisibleHandleGpu = descInfo.GPU;
		D3D12_CPU_DESCRIPTOR_HANDLE uavHandle = descInfoCpu.CPU;

		// Create demanded views for each resource
		for (const auto& res : resourcesInfo)
		{
			if (res.Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
			{
				D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
				rtvDesc.Format = res.Desc.Format;
				if (res.Desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
				{
					rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
					rtvDesc.Texture3D.MipSlice = 0;
					rtvDesc.Texture3D.FirstWSlice = 0;
					rtvDesc.Texture3D.WSize = res.Desc.DepthOrArraySize;
				}
				else if (res.Desc.DepthOrArraySize > 1)
				{
					rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
					rtvDesc.Texture2DArray.MipSlice = 0;
					rtvDesc.Texture2DArray.FirstArraySlice = 0;
					rtvDesc.Texture2DArray.ArraySize = res.Desc.DepthOrArraySize;
					rtvDesc.Texture2DArray.PlaneSlice = 0;
				}
				else
				{
					rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
					rtvDesc.Texture2D.MipSlice = 0;
					rtvDesc.Texture2D.PlaneSlice = 0;
				}
				ZE_DX_THROW_FAILED_INFO(device->CreateRenderTargetView(resources[res.Handle].Resource.Get(), &rtvDesc, rtvHandle));
				rtvDsvHandles[res.Handle] = rtvHandle;
				rtvHandle.ptr += rtvDescSize;

				// Generate RT views for proper mips
				if (res.Desc.MipLevels > 1)
				{
					auto& targetResourceMip = rtvDsvMips[res.Handle - 1];
					targetResourceMip = new D3D12_CPU_DESCRIPTOR_HANDLE[res.Desc.MipLevels];
					targetResourceMip[0] = rtvDsvHandles[res.Handle];
					for (U16 i = 1; i < res.Desc.MipLevels; ++i)
					{
						if (res.Desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
							rtvDesc.Texture3D.MipSlice = i;
						else if (res.Desc.DepthOrArraySize > 1)
							rtvDesc.Texture2DArray.MipSlice = i;
						else
							rtvDesc.Texture2D.MipSlice = i;

						ZE_DX_THROW_FAILED_INFO(device->CreateRenderTargetView(resources[res.Handle].Resource.Get(), &rtvDesc, rtvHandle));
						targetResourceMip[i] = rtvHandle;
						rtvHandle.ptr += rtvDescSize;
					}
				}
			}
			else if (res.Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
			{
				D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
				dsvDesc.Format = res.Desc.Format;
				dsvDesc.Flags = D3D12_DSV_FLAG_NONE; // Maybe check if format is DepthOnly so Stencil would be set to read only
				if (res.Desc.DepthOrArraySize > 1)
				{
					dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
					dsvDesc.Texture2DArray.MipSlice = 0;
					dsvDesc.Texture2DArray.FirstArraySlice = 0;
					dsvDesc.Texture2DArray.ArraySize = res.Desc.DepthOrArraySize;
				}
				else
				{
					dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
					dsvDesc.Texture2D.MipSlice = 0;
				}
				ZE_DX_THROW_FAILED_INFO(device->CreateDepthStencilView(resources[res.Handle].Resource.Get(), &dsvDesc, dsvHandle));
				rtvDsvHandles[res.Handle] = dsvHandle;
				dsvHandle.ptr += dsvDescSize;

				// Generate views for proper mips
				if (res.Desc.MipLevels > 1)
				{
					auto& targetResourceMip = rtvDsvMips[res.Handle - 1];
					targetResourceMip = new D3D12_CPU_DESCRIPTOR_HANDLE[res.Desc.MipLevels];
					targetResourceMip[0] = rtvDsvHandles[res.Handle];
					for (U16 i = 1; i < res.Desc.MipLevels; ++i)
					{
						if (res.Desc.DepthOrArraySize > 1)
							dsvDesc.Texture2DArray.MipSlice = i;
						else
							dsvDesc.Texture2D.MipSlice = i;

						ZE_DX_THROW_FAILED_INFO(device->CreateDepthStencilView(resources[res.Handle].Resource.Get(), &dsvDesc, dsvHandle));
						targetResourceMip[i] = dsvHandle;
						dsvHandle.ptr += dsvDescSize;
					}
				}
			}
			else
				rtvDsvHandles[res.Handle].ptr = UINT64_MAX;
			if ((res.Desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) == 0)
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Format = DX::ConvertDepthFormatToResourceView(res.Desc.Format, res.UseStencilView());
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				if (res.Desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
				{
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
					srvDesc.Texture3D.MostDetailedMip = 0;
					srvDesc.Texture3D.MipLevels = res.Desc.MipLevels;
					srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;
				}
				else if (res.IsCube())
				{
					if (res.Desc.DepthOrArraySize > 6)
					{
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
						srvDesc.TextureCubeArray.MostDetailedMip = 0;
						srvDesc.TextureCubeArray.MipLevels = res.Desc.MipLevels;
						srvDesc.TextureCubeArray.First2DArrayFace = 0;
						srvDesc.TextureCubeArray.NumCubes = res.Desc.DepthOrArraySize / 6;
						srvDesc.TextureCubeArray.ResourceMinLODClamp = 0.0f;
					}
					else
					{
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
						srvDesc.TextureCube.MostDetailedMip = 0;
						srvDesc.TextureCube.MipLevels = res.Desc.MipLevels;
						srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
					}
				}
				else if (res.Desc.DepthOrArraySize > 1)
				{
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
					srvDesc.Texture2DArray.MostDetailedMip = 0;
					srvDesc.Texture2DArray.MipLevels = res.Desc.MipLevels;
					srvDesc.Texture2DArray.FirstArraySlice = 0;
					srvDesc.Texture2DArray.ArraySize = res.Desc.DepthOrArraySize;
					srvDesc.Texture2DArray.PlaneSlice = 0;
					srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
				}
				else
				{
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					srvDesc.Texture2D.MostDetailedMip = 0;
					srvDesc.Texture2D.MipLevels = res.Desc.MipLevels;
					srvDesc.Texture2D.PlaneSlice = 0;
					srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
				}
				ZE_DX_THROW_FAILED_INFO(device->CreateShaderResourceView(resources[res.Handle].Resource.Get(), &srvDesc, srvUavShaderVisibleHandle));
				srvHandles[res.Handle] = { srvUavShaderVisibleHandle, srvUavShaderVisibleHandleGpu };
				srvUavShaderVisibleHandle.ptr += srvUavDescSize;
				srvUavShaderVisibleHandleGpu.ptr += srvUavDescSize;
			}
			else
				srvHandles[res.Handle].CpuShaderVisibleHandle.ptr = srvHandles[res.Handle].GpuShaderVisibleHandle.ptr = UINT64_MAX;
		}
		// Split processing so UAV and SRV descriptors are placed next to each other
		for (const auto& res : resourcesInfo)
		{
			if (res.Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = DX::ConvertDepthFormatToResourceView(res.Desc.Format, res.UseStencilView());
				if (res.Desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
				{
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
					uavDesc.Texture3D.MipSlice = 0;
					uavDesc.Texture3D.FirstWSlice = 0;
					uavDesc.Texture3D.WSize = res.Desc.DepthOrArraySize;
				}
				else if (res.Desc.DepthOrArraySize > 1)
				{
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
					uavDesc.Texture2DArray.MipSlice = 0;
					uavDesc.Texture2DArray.FirstArraySlice = 0;
					uavDesc.Texture2DArray.ArraySize = res.Desc.DepthOrArraySize;
					uavDesc.Texture2DArray.PlaneSlice = 0;
				}
				else
				{
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
					uavDesc.Texture2D.MipSlice = 0;
					uavDesc.Texture2D.PlaneSlice = 0;
				}
				ZE_DX_THROW_FAILED_INFO(device->CreateUnorderedAccessView(resources[res.Handle].Resource.Get(), nullptr, &uavDesc, uavHandle));
				device->CopyDescriptorsSimple(1, srvUavShaderVisibleHandle, uavHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				uavHandles[res.Handle - 1] = { uavHandle, srvUavShaderVisibleHandle, srvUavShaderVisibleHandleGpu };
				uavHandle.ptr += srvUavDescSize;
				srvUavShaderVisibleHandle.ptr += srvUavDescSize;
				srvUavShaderVisibleHandleGpu.ptr += srvUavDescSize;

				// Generate views for proper mips
				if (res.Desc.MipLevels > 1)
				{
					auto& targetResourceMip = uavMips[res.Handle - 1];
					targetResourceMip = new HandleUAV[res.Desc.MipLevels];
					targetResourceMip[0] = uavHandles[res.Handle - 1];

					D3D12_CPU_DESCRIPTOR_HANDLE dstStart = srvUavShaderVisibleHandle;
					D3D12_CPU_DESCRIPTOR_HANDLE srcStart = uavHandle;
					for (U16 i = 1; i < res.Desc.MipLevels; ++i)
					{
						if (res.Desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
							uavDesc.Texture3D.MipSlice = i;
						else if (res.Desc.DepthOrArraySize > 1)
							uavDesc.Texture2DArray.MipSlice = i;
						else
							uavDesc.Texture2D.MipSlice = i;

						ZE_DX_THROW_FAILED_INFO(device->CreateUnorderedAccessView(resources[res.Handle].Resource.Get(), nullptr, &uavDesc, uavHandle));
						targetResourceMip[i] = { uavHandle, srvUavShaderVisibleHandle, srvUavShaderVisibleHandleGpu };
						uavHandle.ptr += srvUavDescSize;
						srvUavShaderVisibleHandle.ptr += srvUavDescSize;
						srvUavShaderVisibleHandleGpu.ptr += srvUavDescSize;
					}
					device->CopyDescriptorsSimple(res.Desc.MipLevels - 1, dstStart, srcStart, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				}
			}
			else
				uavHandles[res.Handle - 1].CpuHandle.ptr = uavHandles[res.Handle - 1].CpuShaderVisibleHandle.ptr = uavHandles[res.Handle - 1].GpuShaderVisibleHandle.ptr = UINT64_MAX;
		}
	}

	FrameBuffer::~FrameBuffer()
	{
		ZE_ASSERT_FREED(descInfo.Handle == nullptr && descInfoCpu.Handle == nullptr
			&& rtvDescHeap == nullptr && dsvDescHeap == nullptr
			&& mainHeap == nullptr && uavHeap == nullptr);

		if (resources)
			resources.DeleteArray();
		if (rtvDsvHandles)
			rtvDsvHandles.DeleteArray();
		if (srvHandles)
			srvHandles.DeleteArray();
		if (uavHandles)
			uavHandles.DeleteArray();
		if (rtvDsvMips)
		{
			for (RID i = 0; i < resourceCount - 1; ++i)
				if (rtvDsvMips[i])
					rtvDsvMips[i].DeleteArray();
			rtvDsvMips.DeleteArray();
		}
		if (uavMips)
		{
			for (RID i = 0; i < resourceCount - 1; ++i)
				if (uavMips[i])
					uavMips[i].DeleteArray();
			uavMips.DeleteArray();
		}
	}

	void FrameBuffer::BeginRasterSparse(GFX::CommandList& cl, const RID* rtv, U8 count) const noexcept
	{
		ZE_ASSERT(count <= Settings::MAX_RENDER_TARGETS, "Exceeding max number of concurrently bound render targets!");
		EnterRaster();

		D3D12_CPU_DESCRIPTOR_HANDLE handles[Settings::MAX_RENDER_TARGETS];
		D3D12_VIEWPORT vieports[Settings::MAX_RENDER_TARGETS];
		D3D12_RECT scissorRects[Settings::MAX_RENDER_TARGETS];
		U8 realCount = 0;
		for (U8 i = 0; i < count; ++i)
		{
			RID id = rtv[i];
			if (id != INVALID_RID)
			{
				ZE_ASSERT(id < resourceCount, "Resource ID outside available range!");

				handles[realCount] = rtvDsvHandles[id];
				ZE_ASSERT(handles[realCount].ptr != UINT64_MAX, "Current resource is not suitable for being render target!");

				SetupViewport(vieports[realCount], scissorRects[realCount], id);
				++realCount;
			}
		}
		cl.Get().dx12.GetList()->RSSetViewports(realCount, vieports);
		cl.Get().dx12.GetList()->RSSetScissorRects(realCount, scissorRects);
		cl.Get().dx12.GetList()->OMSetRenderTargets(realCount, handles, false, nullptr);
	}

	void FrameBuffer::BeginRasterSparse(GFX::CommandList& cl, const RID* rtv, RID dsv, U8 count) const noexcept
	{
		ZE_ASSERT(count <= Settings::MAX_RENDER_TARGETS, "Exceeding max number of concurrently bound render targets!");
		ZE_ASSERT(dsv < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(dsv != BACKBUFFER_RID, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(rtvDsvHandles[dsv].ptr != UINT64_MAX, "Current resource is not suitable for being depth stencil!");
		EnterRaster();

		D3D12_CPU_DESCRIPTOR_HANDLE handles[Settings::MAX_RENDER_TARGETS];
		D3D12_VIEWPORT vieports[Settings::MAX_RENDER_TARGETS];
		D3D12_RECT scissorRects[Settings::MAX_RENDER_TARGETS];
		U8 realCount = 0;
		for (U8 i = 0; i < count; ++i)
		{
			RID id = rtv[i];
			if (id != INVALID_RID)
			{
				ZE_ASSERT(id < resourceCount, "Resource ID outside available range!");

				handles[realCount] = rtvDsvHandles[id];
				ZE_ASSERT(handles[i].ptr != UINT64_MAX, "Current resource is not suitable for being render target!");

				SetupViewport(vieports[realCount], scissorRects[realCount], id);
				++realCount;
			}
		}
		cl.Get().dx12.GetList()->RSSetViewports(count, vieports);
		cl.Get().dx12.GetList()->RSSetScissorRects(count, scissorRects);
		cl.Get().dx12.GetList()->OMSetRenderTargets(count, handles, false, rtvDsvHandles + dsv);
	}

	void FrameBuffer::BeginRasterDepthOnly(GFX::CommandList& cl, RID dsv) const noexcept
	{
		ZE_ASSERT(dsv < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(dsv != BACKBUFFER_RID, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(rtvDsvHandles[dsv].ptr != UINT64_MAX, "Current resource is not suitable for being depth stencil!");

		EnterRaster();
		SetViewport(cl.Get().dx12, dsv);
		cl.Get().dx12.GetList()->OMSetRenderTargets(0, nullptr, true, rtvDsvHandles + dsv);
	}

	void FrameBuffer::BeginRaster(GFX::CommandList& cl, RID rtv) const noexcept
	{
		ZE_ASSERT(rtv < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rtvDsvHandles[rtv].ptr != UINT64_MAX, "Current resource is not suitable for being render target!");

		EnterRaster();
		SetViewport(cl.Get().dx12, rtv);
		cl.Get().dx12.GetList()->OMSetRenderTargets(1, rtvDsvHandles + rtv, true, nullptr);
	}

	void FrameBuffer::BeginRasterDepth(GFX::CommandList& cl, RID rtv, RID dsv) const noexcept
	{
		ZE_ASSERT(rtv < resourceCount, "RTV resource ID outside available range!");
		ZE_ASSERT(dsv < resourceCount, "DSV resource ID outside available range!");
		ZE_ASSERT(dsv != BACKBUFFER_RID, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(rtvDsvHandles[rtv].ptr != UINT64_MAX, "Current resource is not suitable for being render target!");
		ZE_ASSERT(rtvDsvHandles[dsv].ptr != UINT64_MAX, "Current resource is not suitable for being depth stencil!");

		EnterRaster();
		SetViewport(cl.Get().dx12, rtv);
		cl.Get().dx12.GetList()->OMSetRenderTargets(1, rtvDsvHandles + rtv, true, rtvDsvHandles + dsv);
	}

	void FrameBuffer::BeginRasterDepthOnly(GFX::CommandList& cl, RID dsv, U16 mipLevel) const noexcept
	{
		ZE_ASSERT(dsv < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(dsv != BACKBUFFER_RID, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(rtvDsvHandles[dsv].ptr != UINT64_MAX, "Current resource is not suitable for being depth stencil!");
		ZE_ASSERT(rtvDsvMips != nullptr, "Mips not supported as no resource has been created with mips greater than 1!");
		ZE_ASSERT(rtvDsvMips[dsv - 1] != nullptr, "Mips for current resource not supported!");
		ZE_ASSERT(mipLevel < GetMipCount(dsv), "Mip level outside available range!");

		EnterRaster();
		SetViewport(cl.Get().dx12, dsv);
		cl.Get().dx12.GetList()->OMSetRenderTargets(0, nullptr, true, rtvDsvMips[dsv - 1] + mipLevel);
	}

	void FrameBuffer::BeginRaster(GFX::CommandList& cl, RID rtv, U16 mipLevel) const noexcept
	{
		ZE_ASSERT(rtv < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rtvDsvHandles[rtv].ptr != UINT64_MAX, "Current resource is not suitable for being render target!");
		ZE_ASSERT(rtvDsvMips != nullptr, "Mips not supported as no resource has been created with mips greater than 1!");
		ZE_ASSERT(rtvDsvMips[rtv - 1] != nullptr, "Mips for current resource not supported!");
		ZE_ASSERT(mipLevel < GetMipCount(rtv), "Mip level outside available range!");

		EnterRaster();
		SetViewport(cl.Get().dx12, rtv);
		cl.Get().dx12.GetList()->OMSetRenderTargets(1, rtvDsvMips[rtv - 1] + mipLevel, true, nullptr);
	}

	void FrameBuffer::BeginRasterDepth(GFX::CommandList& cl, RID rtv, RID dsv, U16 mipLevel) const noexcept
	{
		ZE_ASSERT(rtv < resourceCount, "RTV resource ID outside available range!");
		ZE_ASSERT(dsv < resourceCount, "DSV resource ID outside available range!");
		ZE_ASSERT(dsv != BACKBUFFER_RID, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(rtvDsvHandles[rtv].ptr != UINT64_MAX, "Current resource is not suitable for being render target!");
		ZE_ASSERT(rtvDsvHandles[dsv].ptr != UINT64_MAX, "Current resource is not suitable for being depth stencil!");
		ZE_ASSERT(rtvDsvMips != nullptr, "Mips not supported as no resource has been created with mips greater than 1!");
		ZE_ASSERT(rtvDsvMips[rtv - 1] != nullptr, "Mips for current RTV resource not supported!");
		ZE_ASSERT(rtvDsvMips[dsv - 1] != nullptr, "Mips for current DSV resource not supported!");
		ZE_ASSERT(mipLevel < GetMipCount(rtv), "Mip level outside available RTV range!");
		ZE_ASSERT(mipLevel < GetMipCount(dsv), "Mip level outside available DSV range!");

		EnterRaster();
		SetViewport(cl.Get().dx12, rtv);
		cl.Get().dx12.GetList()->OMSetRenderTargets(1, rtvDsvMips[rtv - 1] + mipLevel, true, rtvDsvMips[dsv - 1] + mipLevel);
	}

	void FrameBuffer::SetSRV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID srv) const noexcept
	{
		ZE_ASSERT(srv < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(GetSRV(srv).GpuShaderVisibleHandle.ptr != UINT64_MAX, "Current resource is not suitable for being shader resource!");

		const auto& schema = bindCtx.BindingSchema.Get().dx12;
		ZE_ASSERT(schema.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::SRV
			|| schema.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::Table,
			"Bind slot is not a shader resource or table! Wrong root signature or order of bindings!");

		auto* list = cl.Get().dx12.GetList();
		if (schema.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::SRV)
		{
			if (schema.IsCompute())
				list->SetComputeRootShaderResourceView(bindCtx.Count++, GetResource(srv)->GetGPUVirtualAddress());
			else
				list->SetGraphicsRootShaderResourceView(bindCtx.Count++, GetResource(srv)->GetGPUVirtualAddress());
		}
		else if (schema.IsCompute())
			list->SetComputeRootDescriptorTable(bindCtx.Count++, GetSRV(srv).GpuShaderVisibleHandle);
		else
			list->SetGraphicsRootDescriptorTable(bindCtx.Count++, GetSRV(srv).GpuShaderVisibleHandle);
	}

	void FrameBuffer::SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID uav) const noexcept
	{
		ZE_ASSERT(uav < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(uav != BACKBUFFER_RID, "Cannot use backbuffer as unnordered access!");
		ZE_ASSERT(GetUAV(uav).GpuShaderVisibleHandle.ptr != UINT64_MAX, "Current resource is not suitable for being unnordered access!");

		const auto& schema = bindCtx.BindingSchema.Get().dx12;
		ZE_ASSERT(schema.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::UAV
			|| schema.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::Table,
			"Bind slot is not a unnordered access or table! Wrong root signature or order of bindings!");

		auto* list = cl.Get().dx12.GetList();
		if (schema.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::UAV)
		{
			if (schema.IsCompute())
				list->SetComputeRootUnorderedAccessView(bindCtx.Count++, GetResource(uav)->GetGPUVirtualAddress());
			else
				list->SetGraphicsRootUnorderedAccessView(bindCtx.Count++, GetResource(uav)->GetGPUVirtualAddress());
		}
		else if (schema.IsCompute())
			list->SetComputeRootDescriptorTable(bindCtx.Count++, GetUAV(uav).GpuShaderVisibleHandle);
		else
			list->SetGraphicsRootDescriptorTable(bindCtx.Count++, GetUAV(uav).GpuShaderVisibleHandle);
	}

	void FrameBuffer::SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID uav, U16 mipLevel) const noexcept
	{
		ZE_ASSERT(uav < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(uav != BACKBUFFER_RID, "Cannot use backbuffer as unnordered access!");
		ZE_ASSERT(GetUAV(uav).GpuShaderVisibleHandle.ptr != UINT64_MAX, "Current resource is not suitable for being unnordered access!");
		ZE_ASSERT(uavMips != nullptr, "Mips not supported as no UAV resource has been created with mips greater than 1!");
		ZE_ASSERT(uavMips[uav - 1] != nullptr, "Mips for current resource not supported!");
		ZE_ASSERT(mipLevel < GetMipCount(uav), "Mip level outside available UAV range!");

		const auto& schema = bindCtx.BindingSchema.Get().dx12;
		ZE_ASSERT(schema.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::Table,
			"Bind slot is not a table! Wrong root signature or order of bindings!");

		auto* list = cl.Get().dx12.GetList();
		if (schema.IsCompute())
			list->SetComputeRootDescriptorTable(bindCtx.Count++, uavMips[uav - 1][mipLevel].GpuShaderVisibleHandle);
		else
			list->SetGraphicsRootDescriptorTable(bindCtx.Count++, uavMips[uav - 1][mipLevel].GpuShaderVisibleHandle);
	}

	void FrameBuffer::EndRaster(GFX::CommandList& cl) const noexcept
	{
#if !_ZE_MODE_RELEASE
		ZE_ASSERT(isRasterActive, "Calling EndRaster() while not in rasterization mode!");

		isRasterActive = false;
#endif
	}

	void FrameBuffer::ClearRTV(GFX::CommandList& cl, RID rtv, const ColorF4& color) const noexcept
	{
		ZE_ASSERT(rtv < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rtvDsvHandles[rtv].ptr != UINT64_MAX, "Current resource is not suitable for being render target!");

		cl.Get().dx12.GetList()->ClearRenderTargetView(rtvDsvHandles[rtv],
			reinterpret_cast<const float*>(&color), 0, nullptr);
	}

	void FrameBuffer::ClearDSV(GFX::CommandList& cl, RID dsv, float depth, U8 stencil) const noexcept
	{
		ZE_ASSERT(dsv < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(dsv != BACKBUFFER_RID, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(rtvDsvHandles[dsv].ptr != UINT64_MAX, "Current resource is not suitable for being depth stencil!");

		cl.Get().dx12.GetList()->ClearDepthStencilView(rtvDsvHandles[dsv],
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depth, stencil, 0, nullptr);
	}

	void FrameBuffer::ClearUAV(GFX::CommandList& cl, RID uav, const ColorF4& color) const noexcept
	{
		ZE_ASSERT(uav < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(uav != BACKBUFFER_RID, "Cannot use backbuffer as unnordered access!");
		ZE_ASSERT(GetUAV(uav).CpuHandle.ptr != UINT64_MAX, "Current resource is not suitable for being unnordered access!");

		const HandleUAV& desc = GetUAV(uav);
		cl.Get().dx12.GetList()->ClearUnorderedAccessViewFloat(desc.GpuShaderVisibleHandle, desc.CpuHandle,
			GetResource(uav).Get(), reinterpret_cast<const float*>(&color), 0, nullptr);
	}

	void FrameBuffer::ClearUAV(GFX::CommandList& cl, RID uav, const Pixel colors[4]) const noexcept
	{
		ZE_ASSERT(uav < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(uav != BACKBUFFER_RID, "Cannot use backbuffer as unnordered access!");
		ZE_ASSERT(GetUAV(uav).CpuHandle.ptr != UINT64_MAX, "Current resource is not suitable for being unnordered access!");

		const HandleUAV& desc = GetUAV(uav);
		cl.Get().dx12.GetList()->ClearUnorderedAccessViewUint(desc.GpuShaderVisibleHandle, desc.CpuHandle,
			GetResource(uav).Get(), reinterpret_cast<const U32*>(colors), 0, nullptr);
	}

	void FrameBuffer::Copy(GFX::CommandList& cl, RID src, RID dest) const noexcept
	{
		ZE_ASSERT(src < resourceCount, "Source resource ID outside available range!");
		ZE_ASSERT(dest < resourceCount, "Destination resource ID outside available range!");
		ZE_ASSERT(GetDimmensions(src) == GetDimmensions(dest), "Resources must have same dimmensions for copy!");

		cl.Get().dx12.GetList()->CopyResource(GetResource(dest).Get(), GetResource(src).Get());
	}

	void FrameBuffer::CopyBufferRegion(GFX::CommandList& cl, RID src, U64 srcOffset, RID dest, U64 destOffset, U64 bytes) const noexcept
	{
		ZE_ASSERT(src < resourceCount, "Source resource ID outside available range!");
		ZE_ASSERT(dest < resourceCount, "Destination resource ID outside available range!");
		ZE_ASSERT(srcOffset + bytes <= GetDimmensions(src).X, "Source copy region outside of resource!");
		ZE_ASSERT(destOffset + bytes <= GetDimmensions(dest).X, "Destination copy region outside of resource!");

		cl.Get().dx12.GetList()->CopyBufferRegion(GetResource(dest).Get(), destOffset,
			GetResource(src).Get(), srcOffset, bytes);
	}

	void FrameBuffer::Barrier(GFX::CommandList& cl, const GFX::Pipeline::BarrierTransition* barriers, U32 count) const noexcept
	{
		ZE_ASSERT(barriers, "Empty barriers to perform!");
		ZE_ASSERT(count > 0, "No barriers to perform!");

		std::unique_ptr<D3D12_TEXTURE_BARRIER[]> texBarriers = std::make_unique<D3D12_TEXTURE_BARRIER[]>(count);
		for (U32 i = 0; i < count; ++i)
			FillBarier(texBarriers[i], barriers[i]);
		PerformBarrier(cl.Get().dx12, texBarriers.get(), count);
	}

	void FrameBuffer::Barrier(GFX::CommandList& cl, const GFX::Pipeline::BarrierTransition& desc) const noexcept
	{
		D3D12_TEXTURE_BARRIER barrier;
		FillBarier(barrier, desc);
		PerformBarrier(cl.Get().dx12, &barrier, 1);
	}

	void FrameBuffer::ExecuteXeSS(GFX::Device& dev, GFX::CommandList& cl, RID color, RID motionVectors, RID depth,
		RID exposure, RID responsive, RID output, float jitterX, float jitterY, bool reset) const
	{
		ZE_ASSERT(color < resourceCount, "Color resource ID outside available range!");
		ZE_ASSERT(motionVectors < resourceCount, "Motion vectors resource ID outside available range!");
		ZE_ASSERT(output < resourceCount, "XeSS output resource ID outside available range!");
		ZE_XESS_ENABLE();

		xess_d3d12_execute_params_t execParams = {};
		execParams.pColorTexture = GetResource(color).Get();
		execParams.pVelocityTexture = GetResource(motionVectors).Get();
		execParams.pDepthTexture = depth != INVALID_RID ? GetResource(depth).Get() : nullptr;
		execParams.pExposureScaleTexture = exposure != INVALID_RID ? GetResource(exposure).Get() : nullptr;
		execParams.pResponsivePixelMaskTexture = responsive != INVALID_RID ? GetResource(responsive).Get() : nullptr;
		execParams.pOutputTexture = GetResource(output).Get();

		UInt2 renderSize = GetDimmensions(color);
		execParams.jitterOffsetX = Data::GetUnitPixelJitterX(jitterX, renderSize.X);
		execParams.jitterOffsetY = Data::GetUnitPixelJitterY(jitterY, renderSize.Y);
		execParams.exposureScale = 1.0f;
		execParams.resetHistory = static_cast<U32>(reset);
		execParams.inputWidth = renderSize.X;
		execParams.inputHeight = renderSize.Y;
		execParams.inputColorBase = { 0, 0 };
		execParams.inputMotionVectorBase = { 0, 0 };
		execParams.inputDepthBase = { 0, 0 };
		execParams.inputResponsiveMaskBase = { 0, 0 };
		execParams.outputColorBase = { 0, 0 };
		execParams.pDescriptorHeap = nullptr; // TODO: When external heap, specify allocated descriptors here
		execParams.descriptorHeapOffset = 0;
		ZE_XESS_THROW_FAILED(xessD3D12Execute(dev.GetXeSSCtx(), cl.Get().dx12.GetList(), &execParams), "Error performing XeSS!");
	}

	void FrameBuffer::SwapBackbuffer(GFX::Device& dev, GFX::SwapChain& swapChain) noexcept
	{
		auto backbufferRtvSrv = swapChain.Get().dx12.SetCurrentBackbuffer(dev.Get().dx12, resources[BACKBUFFER_RID].Resource);
		rtvDsvHandles[BACKBUFFER_RID] = backbufferRtvSrv.RTV;
		srvHandles[BACKBUFFER_RID].CpuShaderVisibleHandle = backbufferRtvSrv.SRVCpu;
		srvHandles[BACKBUFFER_RID].GpuShaderVisibleHandle = backbufferRtvSrv.SRVGpu;
	}

	void FrameBuffer::Free(GFX::Device& dev) noexcept
	{
		rtvDescHeap = nullptr;
		dsvDescHeap = nullptr;
		mainHeap = nullptr;
		uavHeap = nullptr;
		if (descInfo.Handle)
			dev.Get().dx12.FreeDescs(descInfo);
		if (descInfoCpu.Handle)
			dev.Get().dx12.FreeDescs(descInfoCpu);
	}
}