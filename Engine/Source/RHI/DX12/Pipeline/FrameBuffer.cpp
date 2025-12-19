#include "RHI/DX12/Pipeline/FrameBuffer.h"
#include "Data/Camera.h"
#include "GFX/FfxApiFunctions.h"
#include "GFX/XeSSException.h"

namespace ZE::RHI::DX12::Pipeline
{
#if !_ZE_MODE_RELEASE
	void FrameBuffer::PrintMemory(std::string&& memID, U32 levelCount, U64 heapSize,
		std::vector<ResourceInitInfo>::iterator resBegin, std::vector<ResourceInitInfo>::iterator resEnd,
		const std::vector<std::pair<U32, U32>>& resourcesLifetime) noexcept
	{
		// TODO: Don't go with 4KB chunks since it's making too big image
		auto lastRes = resEnd;
		--lastRes;
		U32 maxChunks = lastRes->ChunkOffset + lastRes->Chunks;

		const U32 pixelsPerLevel = Math::Clamp(maxChunks / levelCount, 240U, 512U);
		const U32 separatorPixels = Math::Clamp(pixelsPerLevel / 20, 8U, 48U);
		const U32 chunkPixels = pixelsPerLevel - separatorPixels;

		GFX::Surface print(levelCount * pixelsPerLevel, maxChunks, PixelFormat::R8G8B8A8_UNorm);
		Pixel* image = reinterpret_cast<Pixel*>(print.GetBuffer());
		const U32 rowWidth = print.GetRowByteSize() / sizeof(Pixel);
		// Clear output image
		for (U32 y = 0; y < print.GetHeight(); ++y)
			for (U32 x = 0; x < print.GetWidth(); ++x)
				image[y * rowWidth + x] = PixelVal::Black;

		// Write regions for all resources
		for (; resBegin != resEnd; ++resBegin)
		{
			// Compute resource color
			const U64 val = resourcesLifetime.size() * resBegin->Handle;
			const Pixel pixel(static_cast<U8>(val >> (val % 3)),
				static_cast<U8>(val >> ((val + 1) % 3)),
				static_cast<U8>(val >> ((val + 2) % 3)));

			// Fill resorce rectangle
			for (U32 chunk = 0; chunk < resBegin->Chunks; ++chunk)
			{
				U32 endLevel = resourcesLifetime.at(resBegin->Handle).second;
				for (U32 level = resourcesLifetime.at(resBegin->Handle).first; level < endLevel; ++level)
				{
					const U64 offset = resBegin->ChunkOffset + chunk + level * pixelsPerLevel;
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
				const U32 offset = chunk * chunkPixels + level * pixelsPerLevel;
				for (U64 p = 0; p < separatorPixels; ++p)
					image[offset + p] = PixelVal::White;
			}
		}
		print.Save("memory_print_dx12_" + memID + "_" + std::to_string(heapSize) + "B.png");
	}
#endif

	U64 FrameBuffer::AllocateResources(std::vector<ResourceInitInfo>::iterator resBegin, std::vector<ResourceInitInfo>::iterator resEnd,
		const std::vector<std::pair<U32, U32>>& resourcesLifetime, U32 levelCount, GFX::Pipeline::FrameBufferFlags flags) noexcept
	{
		U32 heapChunks = 0;

		// Other algorithm: https://stackoverflow.com/questions/25683078/algorithm-for-packing-time-slots
		if (flags & GFX::Pipeline::FrameBufferFlag::NoMemoryAliasing)
		{
			// No resource aliasing so place all of the one after another
			for (; resBegin != resEnd; ++resBegin)
			{
				// Make sure that current offset will be aligned
				heapChunks = Math::AlignUp(heapChunks, Utils::SafeCast<U32>(resBegin->Desc.Alignment / D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT));
				resBegin->ChunkOffset = heapChunks;
				heapChunks += resBegin->Chunks;
			}
		}
		else
		{
			// Find free memory regions for resources
			std::vector<RID> memory;
			U32 allocatedChunks = 0;
			for (auto it = resBegin; it != resEnd; ++it)
			{
				const U32 chunkAlignment = Utils::SafeCast<U32>(it->Desc.Alignment / D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT);
				// TODO: maybe treat temporals differently in terms of search
				// (push at the front or end but what if different alignments)
				U32 startLevel = 0, endLevel = levelCount;
				if (!it->IsTemporal())
				{
					startLevel = resourcesLifetime.at(it->Handle).first;
					endLevel = resourcesLifetime.at(it->Handle).second;
				}

				// Search through whole memory
				U32 foundOffset = UINT32_MAX, chunksFound = 0;
				for (U32 offset = 0; offset < allocatedChunks; offset = Math::AlignUp(++offset, chunkAlignment))
				{
					if (foundOffset == UINT32_MAX)
						foundOffset = offset;
					// Check chunks for whole requested duration
					for (U32 time = startLevel; time < endLevel; ++time)
					{
						if (memory.at(offset * levelCount + time) != INVALID_RID)
						{
							foundOffset = UINT32_MAX;
							break;
						}
					}
					if (foundOffset != UINT32_MAX)
					{
						if (++chunksFound == it->Chunks)
							break;
					}
					else
					{
						chunksFound = 0;
						foundOffset = UINT32_MAX;
					}
				}

				// Allocate new heap chunks
				if (foundOffset == UINT32_MAX || chunksFound != it->Chunks)
				{
					foundOffset = Math::AlignUp(allocatedChunks, chunkAlignment);
					allocatedChunks = foundOffset + it->Chunks;
					memory.resize(allocatedChunks * levelCount);
				}

				// Reserve space in memoryv
				for (U32 chunk = 0; chunk < it->Chunks; ++chunk)
					std::fill_n(memory.begin() + Utils::SafeCast<U64>(foundOffset + chunk) * levelCount + startLevel, endLevel - startLevel, it->Handle);
				it->ChunkOffset = foundOffset;
			}
			heapChunks = Utils::SafeCast<U32>(memory.size() / levelCount);
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

	void FrameBuffer::FillBarier(D3D12_BUFFER_BARRIER& barrier, const GFX::Pipeline::BarrierTransition& desc) const noexcept
	{
		ZE_ASSERT(desc.Resource < resourceCount, "Resource ID outside available range!");

		barrier.SyncBefore = GetBarrierSync(desc.StageBefore);
		barrier.SyncAfter = GetBarrierSync(desc.StageAfter);
		switch (desc.Type)
		{
		default:
		ZE_ENUM_UNHANDLED();
		case GFX::Pipeline::BarrierType::Immediate:
		break;
		case GFX::Pipeline::BarrierType::SplitBegin:
		barrier.SyncAfter = D3D12_BARRIER_SYNC_SPLIT;
		break;
		case GFX::Pipeline::BarrierType::SplitEnd:
		barrier.SyncBefore = D3D12_BARRIER_SYNC_SPLIT;
		break;
		}
		barrier.AccessBefore = GetBarrierAccess(desc.AccessBefore);
		barrier.AccessAfter = GetBarrierAccess(desc.AccessAfter);
		barrier.pResource = GetResource(desc.Resource).Get();
		barrier.Offset = 0;
		barrier.Size = UINT64_MAX;
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
			desc.Resource != BACKBUFFER_RID && desc.LayoutBefore == GFX::Pipeline::TextureLayout::DepthStencilWrite && rtvDsvHandles[desc.Resource].ptr != UINT64_MAX,
			"Current resource is not suitable for being depth stencil!");
		ZE_ASSERT(desc.LayoutAfter != GFX::Pipeline::TextureLayout::DepthStencilWrite ||
			desc.Resource != BACKBUFFER_RID && desc.LayoutAfter == GFX::Pipeline::TextureLayout::DepthStencilWrite && rtvDsvHandles[desc.Resource].ptr != UINT64_MAX,
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
		barrier.SyncAfter = D3D12_BARRIER_SYNC_SPLIT;
		break;
		case GFX::Pipeline::BarrierType::SplitEnd:
		barrier.SyncBefore = D3D12_BARRIER_SYNC_SPLIT;
		break;
		}
		barrier.AccessBefore = GetBarrierAccess(desc.AccessBefore);
		barrier.AccessAfter = GetBarrierAccess(desc.AccessAfter);
		barrier.LayoutBefore = GetBarrierLayout(desc.LayoutBefore);
		barrier.LayoutAfter = GetBarrierLayout(desc.LayoutAfter);
		barrier.pResource = GetResource(desc.Resource).Get();
		barrier.Subresources.IndexOrFirstMipLevel = desc.Subresource;
		barrier.Subresources.NumMipLevels = 0;
		barrier.Subresources.FirstArraySlice = 0;
		barrier.Subresources.NumArraySlices = 0;
		barrier.Subresources.FirstPlane = 0;
		barrier.Subresources.NumPlanes = 0;
		barrier.Flags = desc.LayoutBefore == GFX::Pipeline::TextureLayout::Undefined ? D3D12_TEXTURE_BARRIER_FLAG_DISCARD : D3D12_TEXTURE_BARRIER_FLAG_NONE;
	}

	void FrameBuffer::PerformBarrier(CommandList& cl, const D3D12_TEXTURE_BARRIER* barriersTex, U32 countTex, const D3D12_BUFFER_BARRIER* barriersBuff, U32 countBuff) const noexcept
	{
		D3D12_BARRIER_GROUP groups[2];
		U32 groupIndex = 0;
		if (countTex)
		{
			groups[0].Type = D3D12_BARRIER_TYPE_TEXTURE;
			groups[0].NumBarriers = countTex;
			groups[0].pTextureBarriers = barriersTex;
			++groupIndex;
		}
		if (countBuff)
		{
			groups[groupIndex].Type = D3D12_BARRIER_TYPE_BUFFER;
			groups[groupIndex].NumBarriers = countBuff;
			groups[groupIndex].pBufferBarriers = barriersBuff;
			++groupIndex;
		}
		ZE_ASSERT(groupIndex > 0, "At least single barrier must be performed!");

		cl.GetList()->Barrier(groupIndex, groups);
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
		RID srvCount = 0;
		RID uavCount = 0, uavAdditionalMipsCount = 0, memoryOnlyUavCount = 0;
		RID bufferCount = 0;
		RID outsideResourceCount = 0;

		// Get sizes in chunks for resources and their descriptors
		std::vector<ResourceInitInfo> resourcesInfo;
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
			ZE_ASSERT_WARN(res.Flags & GFX::Pipeline::FrameResourceFlag::InternalResourceActive, "Resource don't contain active flag! Redundant memory will be allocated on CPU.");
			if (res.Flags & GFX::Pipeline::FrameResourceFlag::InternalResourceActive)
			{
				resDesc.Dimension = GetDimension(res.Type);

				// Different handling for only memory region reservation
				if (res.Flags & GFX::Pipeline::FrameResourceFlag::NoResourceCreation)
				{
					resDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
					resDesc.Width = res.Sizes.X;
					resDesc.Width |= static_cast<U64>(res.Sizes.Y) << 32;
					resDesc.Height = 0;
					resDesc.DepthOrArraySize = 0;
					resDesc.MipLevels = 0;
					resDesc.Format = DXGI_FORMAT_UNKNOWN;
					resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

					const U64 chunksCount = Math::DivideRoundUp(resDesc.Width, static_cast<U64>(D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT));
					resourcesInfo.emplace_back(i, Utils::SafeCast<U32>(chunksCount), 0U, resDesc).SetMemoryOnlyRegion();
					if (res.Type == GFX::Pipeline::FrameResourceType::Buffer)
						++bufferCount;
					else
					{
						++uavCount;
						++memoryOnlyUavCount;
					}
				}
				else if (res.Flags & GFX::Pipeline::FrameResourceFlag::OutsideResource)
				{
					resDesc.Alignment = 0;
					resDesc.Width = 0;
					resDesc.Height = 0;
					resDesc.DepthOrArraySize = 0;
					resDesc.MipLevels = 0;
					resDesc.Format = DX::GetDXFormat(res.Format);
					resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
					resourcesInfo.emplace_back(i, Utils::SafeCast<U32>(0), 0U, resDesc).SetOutsideResource();
					++outsideResourceCount;
				}
				else
				{
					const UInt2 sizes = res.GetResolutionAdjustedSizes();
					ZE_ASSERT((res.Type == GFX::Pipeline::FrameResourceType::Texture1D && sizes.Y == 1)
						|| res.Type != GFX::Pipeline::FrameResourceType::Texture1D, "Height of the 1D texture must be 1!");

					resDesc.Width = sizes.X;
					resDesc.Height = sizes.Y;
					if (res.Type == GFX::Pipeline::FrameResourceType::Buffer)
					{
						resDesc.Height = 1;
						++bufferCount;
						resDesc.Format = DXGI_FORMAT_UNKNOWN;
						resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
					}
					else
					{
						resDesc.Format = DX::GetTypedDepthDXFormat(res.Format);
						resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
					}

					resDesc.DepthOrArraySize = res.DepthOrArraySize;
					if (res.Type == GFX::Pipeline::FrameResourceType::TextureCube)
						resDesc.DepthOrArraySize *= 6;

					resDesc.MipLevels = res.MipLevels;
					if (!resDesc.MipLevels)
						resDesc.MipLevels = Math::GetMipLevels(sizes.X, sizes.Y);
					if (res.Flags & GFX::Pipeline::FrameResourceFlag::SimultaneousAccess)
						resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
					else
						resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

					D3D12_CLEAR_VALUE clearDesc = {};
					clearDesc.Format = resDesc.Format;
					// Check usage flags
					bool isRT = res.Flags & (GFX::Pipeline::FrameResourceFlag::ForceRTV | GFX::Pipeline::FrameResourceFlag::InternalUsageRenderTarget);
					bool isDS = res.Flags & (GFX::Pipeline::FrameResourceFlag::ForceDSV | GFX::Pipeline::FrameResourceFlag::InternalUsageDepth);
					bool isUA = res.Flags & (GFX::Pipeline::FrameResourceFlag::ForceUAV | GFX::Pipeline::FrameResourceFlag::InternalUsageUnorderedAccess);
					bool isSR = res.Flags & (GFX::Pipeline::FrameResourceFlag::ForceSRV | GFX::Pipeline::FrameResourceFlag::InternalUsageShaderResource);
					if (isRT)
					{
						ZE_ASSERT(resDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER, "Cannot create render target view for buffer resource!");
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
						ZE_ASSERT(resDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER, "Cannot create buffer resource as depth stencil!");
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
					// Can't specify deny SRV flag if not DSV
					if (isSR || !isDS)
						++srvCount;
					else
						resDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

					// Get resource alignment and size in chunks
					resDesc.Alignment = D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
					D3D12_RESOURCE_ALLOCATION_INFO1 allocInfo = {};
					device->GetResourceAllocationInfo2(0, 1, &resDesc, &allocInfo);
					if (allocInfo.Alignment != D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT)
					{
						resDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
						device->GetResourceAllocationInfo2(0, 1, &resDesc, &allocInfo);
					}

					// Create resource entry and fill it with proper info
					const U64 chunksCount = Math::DivideRoundUp(allocInfo.SizeInBytes, static_cast<U64>(D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT));
					auto& info = resourcesInfo.emplace_back(i, Utils::SafeCast<U32>(chunksCount), 0U, resDesc, clearDesc, 0, 0);
					if (res.Type == GFX::Pipeline::FrameResourceType::TextureCube)
						info.SetCube();
					if (res.Flags & GFX::Pipeline::FrameResourceFlag::StencilView)
						info.SetStencilView();
					if (res.Flags & GFX::Pipeline::FrameResourceFlag::RawBufferView)
						info.SetRawBufferView();
					if (res.Flags & GFX::Pipeline::FrameResourceFlag::Temporal)
						info.SetTemporal();
					if (res.Flags & GFX::Pipeline::FrameResourceFlag::ArrayView)
						info.ForceArrayView();
					info.ByteStride = sizes.Y; // In case of buffer resource
				}
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

		RID mainHeapResourceCount = 0;
		// Handle resource types (non RT/DS) depending on present tier level
		if (dev.Get().dx12.GetCurrentAllocTier() == AllocatorGPU::AllocTier::Tier1)
		{
			mainHeapResourceCount = rtvCount + dsvCount;
			RID uavHeapResourceCount = Utils::SafeCast<RID>(resourcesInfo.size()) - (bufferCount + mainHeapResourceCount + outsideResourceCount);
			// Sort resources descending by size but ordering them by heap type: RTV/DSV, UAV, Buffer, OutsideResource
			std::sort(resourcesInfo.begin(), resourcesInfo.end(),
				[](const auto& r1, const auto& r2) -> bool
				{
					U8 groupIndex1 = 0, groupIndex2 = 0;
					if (r1.IsOutsideResource())
						groupIndex1 = 3;
					else if (r1.Desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
						groupIndex1 = 2;
					else if (!(r1.Desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)))
						groupIndex1 = 1;
					if (r2.IsOutsideResource())
						groupIndex2 = 3;
					else if (r2.Desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
						groupIndex2 = 2;
					else if (!(r2.Desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)))
						groupIndex2 = 1;

					if (groupIndex1 == groupIndex2)
						return r1.Chunks > r2.Chunks;
					return groupIndex1 < groupIndex2;
				});

			// Create heap for buffer resources
			if (bufferCount)
			{
				auto begin = resourcesInfo.begin() + mainHeapResourceCount + uavHeapResourceCount;
				auto end = resourcesInfo.begin() + mainHeapResourceCount + uavHeapResourceCount + bufferCount;
				// Find offsets for all resources in this heap and get it's size
				heapDesc.SizeInBytes = AllocateResources(begin, resourcesInfo.end(), desc.ResourceLifetimes, desc.PassLevelCount, desc.Flags);

				heapDesc.Flags |= D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
				ZE_DX_THROW_FAILED(device->CreateHeap1(&heapDesc, nullptr, IID_PPV_ARGS(&bufferHeap)));
				ZE_DX_SET_ID(bufferHeap, "GFX::Pipeline::FrameBuffer heap - Buffer");
				ZE_DX_THROW_FAILED(device->SetResidencyPriority(1, reinterpret_cast<IPageable**>(bufferHeap.GetAddressOf()), &residencyPriority));
				heapDesc.Flags &= ~D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;

#if !_ZE_MODE_RELEASE
				if (desc.Flags & GFX::Pipeline::FrameBufferFlag::DebugMemoryPrint)
					PrintMemory("tier1_buffer", desc.PassLevelCount, heapDesc.SizeInBytes, begin, resourcesInfo.end(), desc.ResourceLifetimes);
#endif
				// Set all resources as using Buffer heap for creation later
				for (; begin != end; ++begin)
					begin->SetHeapBuffer();
			}
			heapDesc.Flags |= D3D12_HEAP_FLAG_DENY_BUFFERS;

			// Create heap for non RT or DS buffers
			if (uavHeapResourceCount)
			{
				auto begin = resourcesInfo.begin() + mainHeapResourceCount;
				auto end = resourcesInfo.begin() + mainHeapResourceCount + uavHeapResourceCount;
				// Find offsets for all resources in this heap and get it's size
				heapDesc.SizeInBytes = AllocateResources(begin, end, desc.ResourceLifetimes, desc.PassLevelCount, desc.Flags);

				heapDesc.Flags |= D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES;
				ZE_DX_THROW_FAILED(device->CreateHeap1(&heapDesc, nullptr, IID_PPV_ARGS(&uavHeap)));
				ZE_DX_SET_ID(uavHeap, "GFX::Pipeline::FrameBuffer heap - UAV");
				ZE_DX_THROW_FAILED(device->SetResidencyPriority(1, reinterpret_cast<IPageable**>(uavHeap.GetAddressOf()), &residencyPriority));
				heapDesc.Flags &= ~D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES;
				heapDesc.Flags |= D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;

#if !_ZE_MODE_RELEASE
				if (desc.Flags & GFX::Pipeline::FrameBufferFlag::DebugMemoryPrint)
					PrintMemory("tier1_uav", desc.PassLevelCount, heapDesc.SizeInBytes, begin, end, desc.ResourceLifetimes);
#endif
				// Set all resources as using UAV heap for creation later
				for (; begin != end; ++begin)
					begin->SetHeapUAV();
			}
		}
		else
		{
			mainHeapResourceCount = Utils::SafeCast<RID>(resourcesInfo.size()) - outsideResourceCount;
			// Sort resources descending by size
			std::sort(resourcesInfo.begin(), resourcesInfo.end(),
				[](const auto& r1, const auto& r2) -> bool
				{
					if (r1.IsOutsideResource() || r2.IsOutsideResource())
						return !r1.IsOutsideResource();
					return r1.Chunks > r2.Chunks;
				});
		}

		// Allocate resources and create main heap
		heapDesc.SizeInBytes = AllocateResources(resourcesInfo.begin(), resourcesInfo.begin() + mainHeapResourceCount, desc.ResourceLifetimes, desc.PassLevelCount, desc.Flags);
		ZE_DX_THROW_FAILED(device->CreateHeap1(&heapDesc, nullptr, IID_PPV_ARGS(&mainHeap)));
		ZE_DX_SET_ID(mainHeap, "GFX::Pipeline::FrameBuffer heap - main");
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
		resources[BACKBUFFER_RID].Size = desc.Resources.front().GetResolutionAdjustedSizes();
		resources[BACKBUFFER_RID].Array = desc.Resources.front().DepthOrArraySize;
		resources[BACKBUFFER_RID].Mips = desc.Resources.front().MipLevels;
		resources[BACKBUFFER_RID].Format = desc.Resources.front().Format;
		for (auto& res : resourcesInfo)
		{
			auto& data = resources[res.Handle];
			if (res.IsMemoryOnlyRegion())
			{
				data.Size = { static_cast<U32>(res.Desc.Width), static_cast<U32>(res.Desc.Width >> 32) };
				data.Array = static_cast<U16>(res.ChunkOffset);
				data.Mips = static_cast<U16>(res.ChunkOffset >> 16);
				data.Format = PixelFormat::Unknown;
				data.Dimenions = res.Desc.Dimension;
				data.SetMemoryOnlyRegion();
			}
			else if (res.IsOutsideResource())
			{
				data.Size = { 0, 0 };
				data.Array = 0;
				data.Mips = 0;
				data.Format = DX::GetFormatFromDX(res.Desc.Format);
				data.Dimenions = res.Desc.Dimension;
				data.SetOutsideResource();
			}
			else
			{
				ZE_DX_THROW_FAILED(device->CreatePlacedResource2(res.IsHeapBuffer() ? bufferHeap.Get() : (res.IsHeapUAV() ? uavHeap.Get() : mainHeap.Get()),
					res.ChunkOffset * D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT, &res.Desc, D3D12_BARRIER_LAYOUT_UNDEFINED,
					res.Desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) ? &res.ClearVal : nullptr,
					0, nullptr, IID_PPV_ARGS(&data.Resource)));
				ZE_DX_SET_ID(data.Resource, "RID_" + std::to_string(res.Handle) + (desc.Resources.at(res.Handle).DebugName.size() ? " " + desc.Resources.at(res.Handle).DebugName : ""));

				data.Size = { Utils::SafeCast<U32>(res.Desc.Width), res.Desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER ? res.ByteStride : res.Desc.Height };
				data.Array = res.Desc.DepthOrArraySize;
				data.Mips = res.Desc.MipLevels;
				data.Format = DX::GetFormatFromDX(res.Desc.Format);
				data.Dimenions = res.Desc.Dimension;

				if (data.Dimenions == D3D12_RESOURCE_DIMENSION_TEXTURE1D
					|| data.Dimenions == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
				{
					if (res.Desc.DepthOrArraySize > 1 || res.IsArrayView())
						data.SetArrayView();
					if (res.IsCube() && data.Dimenions == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
						data.SetCube();
				}
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
		const RID uavDescCount = uavCount + uavAdditionalMipsCount - memoryOnlyUavCount;
		descInfo = dev.Get().dx12.AllocDescs(srvCount + uavDescCount);
		if (uavDescCount)
			descInfoCpu = dev.Get().dx12.AllocDescs(uavDescCount, false);

		D3D12_CPU_DESCRIPTOR_HANDLE srvUavShaderVisibleHandle = descInfo.CPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvUavShaderVisibleHandleGpu = descInfo.GPU;
		D3D12_CPU_DESCRIPTOR_HANDLE uavHandle = descInfoCpu.CPU;

		// Create demanded views for each resource
		for (const auto& res : resourcesInfo)
		{
			if (!res.IsMemoryOnlyRegion() && !res.IsOutsideResource())
			{
				if (res.Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
				{
					D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
					rtvDesc.Format = res.Desc.Format;
					switch (res.Desc.Dimension)
					{
					default:
					case D3D12_RESOURCE_DIMENSION_UNKNOWN:
					ZE_ENUM_UNHANDLED();
					case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
					{
						if (res.Desc.DepthOrArraySize > 1)
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
						break;
					}
					case D3D12_RESOURCE_DIMENSION_BUFFER:
					{
						rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_BUFFER;
						rtvDesc.Buffer.FirstElement = 0;
						rtvDesc.Buffer.NumElements = Utils::SafeCast<U32>(res.Desc.Width);
						break;
					}
					case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
					{
						if (res.Desc.DepthOrArraySize > 1)
						{
							rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
							rtvDesc.Texture1DArray.MipSlice = 0;
							rtvDesc.Texture1DArray.FirstArraySlice = 0;
							rtvDesc.Texture1DArray.ArraySize = res.Desc.DepthOrArraySize;
						}
						else
						{
							rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
							rtvDesc.Texture1D.MipSlice = 0;
						}
						break;
					}
					case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
					{
						rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
						rtvDesc.Texture3D.MipSlice = 0;
						rtvDesc.Texture3D.FirstWSlice = 0;
						rtvDesc.Texture3D.WSize = res.Desc.DepthOrArraySize;
						break;
					}
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
							switch (res.Desc.Dimension)
							{
							case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
							{
								rtvDesc.Texture3D.MipSlice = i;
								break;
							}
							case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
							{
								if (res.Desc.DepthOrArraySize > 1)
									rtvDesc.Texture2DArray.MipSlice = i;
								else
									rtvDesc.Texture2D.MipSlice = i;
								break;
							}
							case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
							{
								if (res.Desc.DepthOrArraySize > 1)
									rtvDesc.Texture1DArray.MipSlice = i;
								else
									rtvDesc.Texture1D.MipSlice = i;
								break;
							}
							default:
							break;
							}

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

					if (res.Desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
					{
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
					}
					else
					{
						if (res.Desc.DepthOrArraySize > 1)
						{
							dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
							dsvDesc.Texture1DArray.MipSlice = 0;
							dsvDesc.Texture1DArray.FirstArraySlice = 0;
							dsvDesc.Texture1DArray.ArraySize = res.Desc.DepthOrArraySize;
						}
						else
						{
							dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
							dsvDesc.Texture1D.MipSlice = 0;
						}
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
							if (res.Desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
							{
								if (res.Desc.DepthOrArraySize > 1)
									dsvDesc.Texture2DArray.MipSlice = i;
								else
									dsvDesc.Texture2D.MipSlice = i;
							}
							else
							{
								if (res.Desc.DepthOrArraySize > 1)
									dsvDesc.Texture1DArray.MipSlice = i;
								else
									dsvDesc.Texture1D.MipSlice = i;
							}

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

					switch (res.Desc.Dimension)
					{
					default:
					case D3D12_RESOURCE_DIMENSION_UNKNOWN:
					ZE_ENUM_UNHANDLED();
					case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
					{
						if (res.IsCube())
						{
							if (res.Desc.DepthOrArraySize > 6 || res.IsArrayView())
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
						else if (res.Desc.DepthOrArraySize > 1 || res.IsArrayView())
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
						break;
					}
					case D3D12_RESOURCE_DIMENSION_BUFFER:
					{
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
						srvDesc.Buffer.FirstElement = 0;
						srvDesc.Buffer.NumElements = Utils::SafeCast<U32>(res.Desc.Width) / res.ByteStride;
						srvDesc.Buffer.StructureByteStride = res.ByteStride;
						srvDesc.Buffer.Flags = res.IsRawBufferView() ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;
						break;
					}
					case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
					{
						if (res.Desc.DepthOrArraySize > 1 || res.IsArrayView())
						{
							srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
							srvDesc.Texture1DArray.MostDetailedMip = 0;
							srvDesc.Texture1DArray.MipLevels = res.Desc.MipLevels;
							srvDesc.Texture1DArray.FirstArraySlice = 0;
							srvDesc.Texture1DArray.ArraySize = res.Desc.DepthOrArraySize;
							srvDesc.Texture1DArray.ResourceMinLODClamp = 0.0f;
						}
						else
						{
							srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
							srvDesc.Texture1D.MostDetailedMip = 0;
							srvDesc.Texture1D.MipLevels = res.Desc.MipLevels;
							srvDesc.Texture1D.ResourceMinLODClamp = 0.0f;
						}
						break;
					}
					case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
					{
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
						srvDesc.Texture3D.MostDetailedMip = 0;
						srvDesc.Texture3D.MipLevels = res.Desc.MipLevels;
						srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;
						break;
					}
					}

					ZE_DX_THROW_FAILED_INFO(device->CreateShaderResourceView(resources[res.Handle].Resource.Get(), &srvDesc, srvUavShaderVisibleHandle));
					srvHandles[res.Handle] = { srvUavShaderVisibleHandle, srvUavShaderVisibleHandleGpu };
					srvUavShaderVisibleHandle.ptr += srvUavDescSize;
					srvUavShaderVisibleHandleGpu.ptr += srvUavDescSize;
				}
				else
					srvHandles[res.Handle].CpuShaderVisibleHandle.ptr = srvHandles[res.Handle].GpuShaderVisibleHandle.ptr = UINT64_MAX;
			}
		}
		// Split processing so UAV and SRV descriptors are placed next to each other
		for (const auto& res : resourcesInfo)
		{
			if (!res.IsMemoryOnlyRegion() && !res.IsOutsideResource())
			{
				if (res.Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
				{
					D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
					uavDesc.Format = DX::ConvertDepthFormatToResourceView(res.Desc.Format, res.UseStencilView());

					switch (res.Desc.Dimension)
					{
					default:
					case D3D12_RESOURCE_DIMENSION_UNKNOWN:
					ZE_ENUM_UNHANDLED();
					case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
					{
						if (res.Desc.DepthOrArraySize > 1 || res.IsArrayView())
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
						break;
					}
					case D3D12_RESOURCE_DIMENSION_BUFFER:
					{
						uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
						uavDesc.Buffer.FirstElement = 0;
						uavDesc.Buffer.NumElements = Utils::SafeCast<U32>(res.Desc.Width) / res.ByteStride;
						uavDesc.Buffer.StructureByteStride = res.ByteStride;
						uavDesc.Buffer.CounterOffsetInBytes = 0;
						uavDesc.Buffer.Flags = res.IsRawBufferView() ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE;
						break;
					}
					case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
					{
						if (res.Desc.DepthOrArraySize > 1 || res.IsArrayView())
						{
							uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
							uavDesc.Texture1DArray.MipSlice = 0;
							uavDesc.Texture1DArray.FirstArraySlice = 0;
							uavDesc.Texture1DArray.ArraySize = res.Desc.DepthOrArraySize;
						}
						else
						{
							uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
							uavDesc.Texture1D.MipSlice = 0;
						}
						break;
					}
					case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
					{
						uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
						uavDesc.Texture3D.MipSlice = 0;
						uavDesc.Texture3D.FirstWSlice = 0;
						uavDesc.Texture3D.WSize = res.Desc.DepthOrArraySize;
						break;
					}
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
							switch (res.Desc.Dimension)
							{
							case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
							{
								uavDesc.Texture3D.MipSlice = i;
								break;
							}
							case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
							{
								if (res.Desc.DepthOrArraySize > 1)
									uavDesc.Texture2DArray.MipSlice = i;
								else
									uavDesc.Texture2D.MipSlice = i;
								break;
							}
							case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
							{
								if (res.Desc.DepthOrArraySize > 1)
									uavDesc.Texture1DArray.MipSlice = i;
								else
									uavDesc.Texture1D.MipSlice = i;
								break;
							}
							default:
							break;
							}

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

		// Finish XeSS initialization with correct regions
		auto xessRes = dev.Get().dx12.GetXeSSAliasableResources();
		if (xessRes.first != INVALID_RID || xessRes.second != INVALID_RID)
		{
			ZE_XESS_ENABLE();

			xess_d3d12_init_params_t initParams = {};
			initParams.outputResolution = dev.Get().dx12.GetXeSSTargetResolution();
			initParams.qualitySetting = dev.Get().dx12.GetXeSSQuality();
			initParams.initFlags = dev.Get().dx12.GetXeSSInitFlags();
			initParams.creationNodeMask = 0;
			initParams.visibleNodeMask = 0;
			if (xessRes.first != INVALID_RID)
			{
				initParams.pTempBufferHeap = dev.Get().dx12.GetCurrentAllocTier() == AllocatorGPU::AllocTier::Tier1 ? bufferHeap.Get() : mainHeap.Get();
				initParams.bufferHeapOffset = static_cast<U64>(GetArraySize(xessRes.first));
				initParams.bufferHeapOffset |= static_cast<U64>(GetMipCount(xessRes.first)) << 16;
				initParams.bufferHeapOffset *= D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
			}
			else
			{
				initParams.pTempBufferHeap = nullptr;
				initParams.bufferHeapOffset = 0;
			}
			if (xessRes.second != INVALID_RID)
			{
				initParams.pTempTextureHeap = dev.Get().dx12.GetCurrentAllocTier() == AllocatorGPU::AllocTier::Tier1 ? uavHeap.Get() : mainHeap.Get();
				initParams.textureHeapOffset = static_cast<U64>(GetArraySize(xessRes.second));
				initParams.textureHeapOffset |= static_cast<U64>(GetMipCount(xessRes.second)) << 16;
				initParams.textureHeapOffset *= D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
			}
			else
			{
				initParams.pTempTextureHeap = nullptr;
				initParams.textureHeapOffset = 0;
			}
			initParams.pPipelineLibrary = nullptr;
			ZE_XESS_THROW_FAILED(xessD3D12Init(dev.Get().dx12.GetXeSSCtx(), &initParams), "Error initializing XeSS D3D12 context!");
		}
	}

	FrameBuffer::~FrameBuffer()
	{
		ZE_ASSERT_FREED(descInfo.Handle == nullptr && descInfoCpu.Handle == nullptr
			&& rtvDescHeap == nullptr && dsvDescHeap == nullptr
			&& mainHeap == nullptr && uavHeap == nullptr && bufferHeap == nullptr
			&& resources == nullptr && rtvDsvHandles == nullptr && srvHandles == nullptr
			&& uavHandles == nullptr && rtvDsvMips == nullptr && uavMips == nullptr);
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

	void FrameBuffer::BeginRaster(GFX::CommandList& cl, RID rtv, RID dsv) const noexcept
	{
		ZE_ASSERT(rtv < resourceCount, "RTV resource ID outside available range!");
		ZE_ASSERT(dsv == INVALID_RID || dsv < resourceCount, "DSV resource ID outside available range!");
		ZE_ASSERT(dsv != BACKBUFFER_RID, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(rtvDsvHandles[rtv].ptr != UINT64_MAX, "Current resource is not suitable for being render target!");
		ZE_ASSERT(dsv == INVALID_RID || rtvDsvHandles[dsv].ptr != UINT64_MAX, "Current resource is not suitable for being depth stencil!");

		EnterRaster();
		SetViewport(cl.Get().dx12, rtv);
		cl.Get().dx12.GetList()->OMSetRenderTargets(1, rtvDsvHandles + rtv, true, dsv != INVALID_RID ? rtvDsvHandles + dsv : nullptr);
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

	void FrameBuffer::BeginRaster(GFX::CommandList& cl, RID rtv, RID dsv, U16 mipLevel) const noexcept
	{
		ZE_ASSERT(rtv < resourceCount, "RTV resource ID outside available range!");
		ZE_ASSERT(dsv == INVALID_RID || dsv < resourceCount, "DSV resource ID outside available range!");
		ZE_ASSERT(dsv != BACKBUFFER_RID, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(rtvDsvHandles[rtv].ptr != UINT64_MAX, "Current resource is not suitable for being render target!");
		ZE_ASSERT(dsv == INVALID_RID || rtvDsvHandles[dsv].ptr != UINT64_MAX, "Current resource is not suitable for being depth stencil!");
		ZE_ASSERT(rtvDsvMips != nullptr, "Mips not supported as no resource has been created with mips greater than 1!");
		ZE_ASSERT(rtvDsvMips[rtv - 1] != nullptr, "Mips for current RTV resource not supported!");
		ZE_ASSERT(dsv == INVALID_RID || rtvDsvMips[dsv - 1] != nullptr, "Mips for current DSV resource not supported!");
		ZE_ASSERT(mipLevel < GetMipCount(rtv), "Mip level outside available RTV range!");
		ZE_ASSERT(dsv == INVALID_RID || mipLevel < GetMipCount(dsv), "Mip level outside available DSV range!");

		EnterRaster();
		SetViewport(cl.Get().dx12, rtv);
		cl.Get().dx12.GetList()->OMSetRenderTargets(1, rtvDsvMips[rtv - 1] + mipLevel, true, dsv != INVALID_RID ? rtvDsvMips[dsv - 1] + mipLevel : nullptr);
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

	void FrameBuffer::SetResourceNGX(NVSDK_NGX_Parameter* param, std::string_view name, RID res) const noexcept
	{
		ZE_ASSERT(res < resourceCount, "Resource ID outside available range!");

		param->Set(name.data(), GetResource(res).Get());
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

	void FrameBuffer::Copy(GFX::Device& dev, GFX::CommandList& cl, RID src, RID dest) const noexcept
	{
		ZE_ASSERT(src < resourceCount, "Source resource ID outside available range!");
		ZE_ASSERT(dest < resourceCount, "Destination resource ID outside available range!");
		ZE_ASSERT(GetDimmensions(src) == GetDimmensions(dest), "Resources must have same dimmensions for copy!");

		IDevice* device = dev.Get().dx12.GetDevice();
		IGraphicsCommandList* list = cl.Get().dx12.GetList();
		IResource* srcRes = GetResource(src).Get();
		IResource* destRes = GetResource(dest).Get();
		D3D12_RESOURCE_DESC1 srcDesc = srcRes->GetDesc1();
		D3D12_RESOURCE_DESC1 destDesc = destRes->GetDesc1();

		if (destDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER || srcDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
		{
			D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
			srcLocation.pResource = srcRes;
			if (srcDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
			{
				srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				srcLocation.SubresourceIndex = 0;
			}
			else
			{
				srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				device->GetCopyableFootprints1(&srcDesc, 0, 1, 0, &srcLocation.PlacedFootprint, nullptr, nullptr, nullptr);
			}

			D3D12_TEXTURE_COPY_LOCATION destLocation = {};
			destLocation.pResource = destRes;
			if (destDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
			{
				destLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				destLocation.SubresourceIndex = 0;
			}
			else
			{
				destLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				device->GetCopyableFootprints1(&destDesc, 0, 1, 0, &destLocation.PlacedFootprint, nullptr, nullptr, nullptr);
			}
			list->CopyTextureRegion(&destLocation, 0, 0, 0, &srcLocation, nullptr);
		}
		else
			list->CopyBufferRegion(destRes, 0, srcRes, 0, destDesc.Width);
	}

	void FrameBuffer::CopyFullResource(GFX::CommandList& cl, RID src, RID dest) const noexcept
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

	void FrameBuffer::InitResource(GFX::CommandList& cl, RID rid, const GFX::Resource::CBuffer& buffer) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(IsBuffer(rid), "Trying to initialize non-buffer resource with CBuffer!");

		cl.Get().dx12.GetList()->CopyResource(GetResource(rid).Get(), buffer.Get().dx12.GetResource());
	}

	void FrameBuffer::InitResource(GFX::CommandList& cl, RID rid, const GFX::Resource::Texture::Pack& texture, U32 index) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(!IsBuffer(rid), "Trying to initialize non-texture resource with texter!");

		cl.Get().dx12.GetList()->CopyResource(GetResource(rid).Get(), texture.Get().dx12.GetResource(index));
	}

	void FrameBuffer::Barrier(GFX::CommandList& cl, const GFX::Pipeline::BarrierTransition* barriers, U32 count) const noexcept
	{
		ZE_ASSERT(barriers, "Empty barriers to perform!");
		ZE_ASSERT(count > 0, "No barriers to perform!");

		std::vector<D3D12_TEXTURE_BARRIER> texBarriers;
		std::vector<D3D12_BUFFER_BARRIER> buffBarriers;
		texBarriers.reserve(count);
		U32 texCount = 0, buffCount = 0;
		for (U32 i = 0; i < count; ++i)
		{
			if (IsBuffer(barriers[i].Resource))
			{
				FillBarier(buffBarriers.emplace_back(), barriers[i]);
				++buffCount;
			}
			else
			{
				FillBarier(texBarriers.emplace_back(), barriers[i]);
				++texCount;
			}
		}
		PerformBarrier(cl.Get().dx12, texBarriers.data(), texCount, buffBarriers.data(), buffCount);
	}

	void FrameBuffer::Barrier(GFX::CommandList& cl, const GFX::Pipeline::BarrierTransition& desc) const noexcept
	{
		D3D12_TEXTURE_BARRIER barrierTex;
		D3D12_BUFFER_BARRIER barrierBuff;
		bool buffer = IsBuffer(desc.Resource);
		if (buffer)
			FillBarier(barrierBuff, desc);
		else
			FillBarier(barrierTex, desc);
		PerformBarrier(cl.Get().dx12, &barrierTex, static_cast<U32>(!buffer), &barrierBuff, static_cast<U32>(buffer));
	}

	void FrameBuffer::RegisterOutsideResource(RID rid, GFX::Resource::Texture::Pack& textures, U32 textureIndex, GFX::Pipeline::FrameResourceType type) noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(resources[rid].IsOutsideResource(), "Trying to register data to incorrect not outside resource!");
		ZE_ASSERT(type != GFX::Pipeline::FrameResourceType::Buffer, "Cannot register buffer resource when passing texture pack!");
		ZE_ASSERT(textures.Get().dx12.GetDescInfo().GpuSide, "Texture descriptors need to be visible for GPU!");

		auto& res = resources[rid];
		res.Resource = textures.Get().dx12.GetRes(textureIndex);

		const auto& resDesc = res.Resource->GetDesc1();
		const auto& texDescInfo = textures.Get().dx12.GetDescInfo();

		res.Size = { Utils::SafeCast<U32>(resDesc.Width), resDesc.Height };
		res.Array = resDesc.DepthOrArraySize;
		res.Mips = resDesc.MipLevels;
		res.Format = DX::GetFormatFromDX(resDesc.Format);
		res.Dimenions = GetDimension(type);

		if (type == GFX::Pipeline::FrameResourceType::TextureCube)
			res.SetCube();
		else if (type != GFX::Pipeline::FrameResourceType::Texture3D && res.Array > 1)
			res.SetArrayView();

		srvHandles[rid].CpuShaderVisibleHandle = texDescInfo.CPU;
		srvHandles[rid].GpuShaderVisibleHandle = texDescInfo.GPU;
	}

	void FrameBuffer::MapResource(GFX::Device& dev, RID rid, void** ptr) const
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_DX_ENABLE(dev.Get().dx12);

		const D3D12_RANGE range = { 0 };
		ZE_DX_THROW_FAILED(GetResource(rid)->Map(0, &range, ptr));
	}

	void FrameBuffer::UnmapResource(RID rid) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		GetResource(rid)->Unmap(0, nullptr);
	}

	FfxApiResource FrameBuffer::GetFfxResource(RID rid, U32 state) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		FfxApiResource resDesc = { nullptr };
		if (rid != INVALID_RID)
		{
			resDesc.resource = GetResource(rid).Get();
			if (IsCubeTexture(rid))
				resDesc.description.type = FFX_API_RESOURCE_TYPE_TEXTURE_CUBE;
			else if (IsTexture1D(rid))
				resDesc.description.type = FFX_API_RESOURCE_TYPE_TEXTURE1D;
			else if (IsTexture3D(rid))
				resDesc.description.type = FFX_API_RESOURCE_TYPE_TEXTURE3D;
			else if (IsBuffer(rid))
				resDesc.description.type = FFX_API_RESOURCE_TYPE_BUFFER;
			else
				resDesc.description.type = FFX_API_RESOURCE_TYPE_TEXTURE2D;

			resDesc.description.format = GFX::GetFfxApiFormat(GetFormat(rid));
			UInt2 sizes = GetDimmensions(rid);
			if (resDesc.description.type == FFX_API_RESOURCE_TYPE_BUFFER)
			{
				resDesc.description.size = sizes.X;
				resDesc.description.stride = sizes.Y;
				resDesc.description.alignment = 0;
			}
			else
			{
				resDesc.description.width = sizes.X;
				resDesc.description.height = sizes.Y;
				resDesc.description.depth = GetArraySize(rid);
			}
			resDesc.description.mipCount = GetMipCount(rid);
			resDesc.description.flags = FFX_API_RESOURCE_FLAGS_NONE;
			resDesc.description.usage = FFX_API_RESOURCE_USAGE_READ_ONLY;
			if (IsUAV(rid))
				resDesc.description.usage = static_cast<FfxApiResourceUsage>(resDesc.description.usage | FFX_API_RESOURCE_USAGE_UAV);
			if (IsArrayView(rid))
				resDesc.description.usage = static_cast<FfxApiResourceUsage>(resDesc.description.usage | FFX_API_RESOURCE_USAGE_ARRAYVIEW);
		}
		resDesc.state = state;
		return resDesc;
	}

	void FrameBuffer::ExecuteXeSS(GFX::Device& dev, GFX::CommandList& cl, RID color, RID motionVectors, RID depth,
		RID exposure, RID responsive, RID output, float jitterX, float jitterY, bool reset) const
	{
		Device& device = dev.Get().dx12;
		ZE_ASSERT(device.IsXeSSEnabled(), "XeSS not enabled!");
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
		execParams.pDescriptorHeap = device.GetDescHeap();
		execParams.descriptorHeapOffset = device.GetXeSSDescriptorsOffset();
		ZE_XESS_THROW_FAILED(xessD3D12Execute(device.GetXeSSCtx(), cl.Get().dx12.GetList(), &execParams), "Error performing XeSS!");
	}

	void FrameBuffer::ExecuteIndirect(GFX::CommandList& cl, GFX::CommandSignature& signature, RID commandsBuffer, U32 commandsOffset) const noexcept
	{
		ZE_ASSERT(commandsBuffer < resourceCount, "Indirect arguments resource ID outside available range!");

		cl.Get().dx12.GetList()->ExecuteIndirect(signature.Get().dx12.GetSignature(), 1, GetResource(commandsBuffer).Get(), commandsOffset, nullptr, 0);
	}

	void FrameBuffer::SwapBackbuffer(GFX::Device& dev, GFX::SwapChain& swapChain) noexcept
	{
		auto backbufferRtvSrv = swapChain.Get().dx12.GetCurrentBackbuffer(dev.Get().dx12, resources[BACKBUFFER_RID].Resource);
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
		bufferHeap = nullptr;
		if (descInfo.Handle)
			dev.Get().dx12.FreeDescs(descInfo);
		if (descInfoCpu.Handle)
			dev.Get().dx12.FreeDescs(descInfoCpu);

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
}