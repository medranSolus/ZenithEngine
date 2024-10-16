#include "RHI/DX12/Pipeline/FrameBuffer.h"

namespace ZE::RHI::DX12::Pipeline
{
	struct ResourceInfo
	{
		RID Handle;
		U32 Offset;
		U32 Chunks;
		U64 StartLevel;
		U64 LastLevel;
		D3D12_RESOURCE_DESC Desc;
		D3D12_CLEAR_VALUE ClearVal;
		DX::ComPtr<IResource> Resource;
		std::bitset<8> Flags;

		constexpr bool IsRTV() const noexcept { return Flags[0]; }
		constexpr bool IsDSV() const noexcept { return Flags[1]; }
		constexpr bool IsSRV() const noexcept { return Flags[2]; }
		constexpr bool IsUAV() const noexcept { return Flags[3]; }
		constexpr bool IsCube() const noexcept { return Flags[4]; }
		constexpr bool UseStencilView() const noexcept { return Flags[5]; }
		constexpr bool IsAliasing() const noexcept { return Flags[6]; }
		void SetAliasing() noexcept { Flags[6] = true; }
		constexpr bool IsTemporal() const noexcept { return Flags[7]; }
		void SetTemporal() noexcept { Flags[7] = true; }
	};

#if _ZE_DEBUG_FRAME_MEMORY_PRINT
	void FrameBuffer::PrintMemory(std::string&& memID, U32 maxChunks, U64 levelCount,
		RID invalidID, const std::vector<RID>& memory, U64 heapSize)
	{
		const U64 pixelsPerLevel = maxChunks / levelCount;
		U64 separatorPixels = pixelsPerLevel / 20;
		if (separatorPixels < 2)
			separatorPixels = 2;
		const U64 chunkPixels = pixelsPerLevel - separatorPixels;
		GFX::Surface print(levelCount * pixelsPerLevel, maxChunks);
		for (U32 chunk = 0; chunk < maxChunks; ++chunk)
		{
			for (U64 level = 0; level < levelCount; ++level)
			{
				U64 val = memory.at(chunk * levelCount + level) * 255 / invalidID;
				Pixel pixel(static_cast<U8>(val >> (8 * (val % 3))),
					static_cast<U8>(val >> (8 * ((val + 1) % 3))),
					static_cast<U8>(val >> (8 * ((val + 2) % 3))),
					static_cast<U8>(val >> 24) ^ 0xFF);
				for (U64 p = 0; p < chunkPixels; ++p)
					print.PutPixel(level * pixelsPerLevel + p, chunk, pixel);
				for (U64 p = 0; p < separatorPixels; ++p)
					print.PutPixel(level * pixelsPerLevel + p + chunkPixels, chunk, { 255, 255, 255, 255 });
			}
		}
		print.Save("memory_print_dx12_" + memID + "_" + std::to_string(heapSize) + "bytes.png");
	}
#endif

#if !_ZE_DEBUG_FRAME_NO_ALIASING_MEMORY
	U64 FrameBuffer::FindHeapSize(U32 maxChunks, U64 levelCount, RID invalidID, const std::vector<RID>& memory) noexcept
	{
		U32 lastChunk = 0;
		for (U32 chunk = 0; chunk < maxChunks; ++chunk)
		{
			bool notFoundInLevel = true;
			for (U64 level = 0; level < levelCount; ++level)
			{
				if (memory.at(chunk * levelCount + level) != invalidID)
				{
					lastChunk = chunk;
					notFoundInLevel = false;
					break;
				}
			}
			if (notFoundInLevel)
				break;
		}
		return Utils::SafeCast<U64>(lastChunk + 1) * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	}

	bool FrameBuffer::CheckResourceAliasing(U32 offset, U32 chunks, U64 startLevel, U64 lastLevel,
		U32 maxChunks, U64 levelCount, RID invalidID, const std::vector<RID>& memory) noexcept
	{
		// Check all chunks of resource
		chunks += offset;
		for (; offset < chunks; ++offset)
		{
			// Time before usage of resource
			for (U64 time = 0; time < startLevel; ++time)
				if (memory.at(offset * levelCount + time) != invalidID)
					return true;
			// Time after usage of resource
			for (++lastLevel; lastLevel < levelCount; ++lastLevel)
				if (memory.at(offset * levelCount + lastLevel) != invalidID)
					return true;
		}
		return false;
	}
#endif

	U32 FrameBuffer::AllocResource(RID id, U32 chunks, U64 startLevel, U64 lastLevel,
		U32 maxChunks, U64 levelCount, RID invalidID, std::vector<RID>& memory)
	{
		U32 foundOffset = 0;
#if _ZE_DEBUG_FRAME_NO_ALIASING_MEMORY
		// Place resources one after another
		for (; foundOffset < maxChunks; ++foundOffset)
			if (memory.at(foundOffset * levelCount) == invalidID)
				break;
		// Indicate that this memory will be occupied by single resource
		for (U32 chunk = 0; chunk < chunks; ++chunk)
			memory.at(Utils::SafeCast<U64>(foundOffset + chunk) * levelCount) = id;
#else
		// Search through whole memory
		for (U32 offset = 0, chunksFound = 0; offset < maxChunks; ++offset)
		{
			// Check chunks for whole requested duration
			for (U64 time = startLevel; time <= lastLevel; ++time)
			{
				if (memory.at(offset * levelCount + time) != invalidID)
				{
					foundOffset = maxChunks;
					break;
				}
			}
			if (foundOffset != maxChunks)
			{
				if (++chunksFound == chunks)
					break;
			}
			else
			{
				chunksFound = 0;
				foundOffset = offset + 1;
			}
		}
#endif
		// Should never happen, bug in memory table creation!!!
		if (foundOffset == maxChunks)
			throw ZE_RGC_EXCEPT("Memory too small to fit requested resource! ID: [" + std::to_string(id) +
				"] Chunks: [" + std::to_string(chunks) + "] Start level: [" + std::to_string(startLevel) +
				"] Last level: [" + std::to_string(lastLevel) + "]");
		// Reserve space in memory
		for (U32 chunk = 0; chunk < chunks; ++chunk)
			std::fill_n(memory.begin() + Utils::SafeCast<U64>(foundOffset + chunk) * levelCount + startLevel, lastLevel - startLevel + 1, id);
		return foundOffset;
	}

	void FrameBuffer::InitResource(CommandList& cl, RID rid) const noexcept
	{
		// Perform discard operations for aliasing resources
		ZE_ASSERT(rid != 0, "Backbuffer do not need discarding it's contents! (Or at least it shouldn't with FLIP_DISCARD... TODO: Check this)");
		if (aliasingResources[rid - 1])
			cl.GetList()->DiscardResource(resources[rid].Resource.Get(), nullptr);
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

	FrameBuffer::FrameBuffer(GFX::Device& dev, GFX::CommandList& mainList, GFX::Pipeline::FrameBufferDesc& desc)
	{
		ZE_DX_ENABLE_ID(dev.Get().dx12);

		resourceCount = Utils::SafeCast<RID>(desc.ResourceInfo.size());
		ZE_ASSERT(desc.ResourceInfo.size() <= UINT16_MAX, "Too much resources, needed wider type!");

		auto* device = dev.Get().dx12.GetDevice();
		std::vector<ResourceInfo> resourcesInfo;
		resourcesInfo.reserve(resourceCount - 1);
		RID rtvCount = 0, rtvAdditionalMipsCount = 0;
		RID dsvCount = 0, dsvAdditionalMipsCount = 0;
		RID srvUavCount = 0;
		RID uavCount = 0, uavAdditionalMipsCount = 0;
		bool rtvDsvMipsPresent = false;
		bool uavMipsPresent = false;

		// Get sizes in chunks for resources and their descriptors
		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		resDesc.SampleDesc.Count = 1;
		resDesc.SampleDesc.Quality = 0;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		for (RID i = 1; i < resourceCount; ++i)
		{
			const auto& res = desc.ResourceInfo.at(i);
			resDesc.Width = res.Sizes.X;
			resDesc.Height = res.Sizes.Y;
			resDesc.DepthOrArraySize = res.ArraySize;
			resDesc.MipLevels = res.MipLevels;
			if (!resDesc.MipLevels)
				resDesc.MipLevels = 1;
			if (res.Flags & GFX::Pipeline::FrameResourceFlags::Cube)
				resDesc.DepthOrArraySize *= 6;
			if (res.Flags & GFX::Pipeline::FrameResourceFlags::SimultaneousAccess)
				resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
			else
				resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			bool isRT = false, isDS = false, isUA = false, isSR = res.Flags & GFX::Pipeline::FrameResourceFlags::ForceSRV;
			if (res.Flags & GFX::Pipeline::FrameResourceFlags::ForceDSV)
			{
				resDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
				isDS = true;
				if (resDesc.MipLevels > 1)
					rtvDsvMipsPresent = true;
			}

			// Check required flags based on lifetime
			const auto& lifetime = desc.ResourceLifetimes.at(i);
			for (const auto& state : lifetime)
			{
				switch (state.second)
				{
				case GFX::Resource::StateRenderTarget:
				{
					ZE_ASSERT(!isDS, "Cannot create depth stencil and render target view for same buffer!");
					ZE_ASSERT(!Utils::IsDepthStencilFormat(res.Format), "Cannot use depth stencil format with render target!");

					resDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
					isRT = true;
					if (resDesc.MipLevels > 1)
						rtvDsvMipsPresent = true;
					break;
				}
				case GFX::Resource::StateDepthRead:
				case GFX::Resource::StateDepthWrite:
				{
					ZE_ASSERT((res.Flags & GFX::Pipeline::FrameResourceFlags::SimultaneousAccess) == 0, "Simultaneous access cannot be used on depth stencil!");
					ZE_ASSERT(!isRT, "Cannot create depth stencil and render target view for same buffer!");
					ZE_ASSERT(!isUA, "Cannot create depth stencil and unordered access view for same buffer!");

					resDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
					isDS = true;
					if (resDesc.MipLevels > 1)
						rtvDsvMipsPresent = true;
					break;
				}
				case GFX::Resource::StateUnorderedAccess:
				{
					ZE_ASSERT(!isDS, "Cannot create depth stencil and unordered access view for same buffer!");

					resDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
					isUA = true;
					if (resDesc.MipLevels > 1)
						uavMipsPresent = true;
					break;
				}
				case GFX::Resource::StateShaderResourcePS:
				case GFX::Resource::StateShaderResourceNonPS:
				case GFX::Resource::StateShaderResourceAll:
				{
					isSR = true;
					break;
				}
				default:
					break;
				}
			}

			resDesc.Format = DX::GetTypedDepthDXFormat(res.Format);
			D3D12_CLEAR_VALUE clearDesc = {};
			clearDesc.Format = resDesc.Format;
			if (Utils::IsDepthStencilFormat(res.Format))
			{
				clearDesc.DepthStencil.Depth = res.ClearDepth;
				clearDesc.DepthStencil.Stencil = res.ClearStencil;
			}
			else
				*reinterpret_cast<ColorF4*>(clearDesc.Color) = res.ClearColor;

			const U64 size = Math::DivideRoundUp(device->GetResourceAllocationInfo(0, 1, &resDesc).SizeInBytes,
				static_cast<U64>(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT));
			auto& info = resourcesInfo.emplace_back(i, 0, Utils::SafeCast<U32>(size), lifetime.begin()->first, lifetime.rbegin()->first, resDesc, clearDesc, nullptr,
				static_cast<U8>(isRT) | (isDS << 1) | (isSR << 2) | (isUA << 3)
				| ((res.Flags & GFX::Pipeline::FrameResourceFlags::Cube) << 4)
				| ((res.Flags & GFX::Pipeline::FrameResourceFlags::StencilView) << 5));
			if (res.Flags & GFX::Pipeline::FrameResourceFlags::Temporal)
				info.SetTemporal();

			if (isRT)
			{
				++rtvCount;
				if (resDesc.MipLevels > 1)
					rtvAdditionalMipsCount += resDesc.MipLevels - 1;
			}
			else if (isDS)
			{
				++dsvCount;
				if (resDesc.MipLevels > 1)
					dsvAdditionalMipsCount += resDesc.MipLevels - 1;
			}
			if (isSR || isUA)
				++srvUavCount;
			if (isUA)
			{
				++uavCount;
				if (resDesc.MipLevels > 1)
					uavAdditionalMipsCount += resDesc.MipLevels - 1;
			}
		}

		// Prepare data for allocating resources and heaps
		if (rtvDsvMipsPresent)
			rtvDsvMips = new Ptr<D3D12_CPU_DESCRIPTOR_HANDLE>[resourceCount - 1];
		if (uavMipsPresent)
			uavMips = new Ptr<std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE>>>[resourceCount - 1];
		const U64 levelCount = desc.TransitionsPerLevel.size() / 2;
		const RID invalidID = resourceCount;
		const D3D12_RESIDENCY_PRIORITY residencyPriority = D3D12_RESIDENCY_PRIORITY_MAXIMUM;
		RID rt_dsCount;
		U32 maxChunks = 0;
		std::vector<RID> memory;
		D3D12_HEAP_DESC heapDesc = {};
		heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapDesc.Properties.CreationNodeMask = 0;
		heapDesc.Properties.VisibleNodeMask = 0;
		heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		heapDesc.Flags = D3D12_HEAP_FLAG_CREATE_NOT_ZEROED;
#if !_ZE_RENDERER_NO_SPLIT_BARRIERS
		std::vector<D3D12_RESOURCE_BARRIER> startingTransitions;
		std::vector<std::pair<U64, D3D12_RESOURCE_BARRIER>> wrappingTransitions;
#endif

		// Handle resource types (Non RT/DS) depending on present tier level
		if (dev.Get().dx12.GetCurrentAllocTier() == AllocatorGPU::AllocTier::Tier1)
		{
			heapDesc.Flags |= D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES;
			// Sort resources descending by size leaving UAV only on the end
			rt_dsCount = rtvCount + dsvCount;
			std::sort(resourcesInfo.begin(), resourcesInfo.end(),
				[](const auto& r1, const auto& r2) -> bool
				{
					if ((r1.IsRTV() || r1.IsDSV()) == (r2.IsRTV() || r2.IsDSV()))
						return r1.Chunks > r2.Chunks;
					return r1.IsRTV() || r1.IsDSV();
				});
			U32 maxChunksUAV = 0;
			for (const auto& res : resourcesInfo)
			{
				if (res.IsRTV() || res.IsDSV())
					maxChunks += res.Chunks;
				else
					maxChunksUAV += res.Chunks;
			}

			// Create heap for non RT or DS buffers
			if (rt_dsCount < resourcesInfo.size())
			{
				// Find free memory regions for UAV only resources
				memory.resize(maxChunksUAV * levelCount, invalidID);
				for (RID i = rt_dsCount; i < resourcesInfo.size(); ++i)
				{
					auto& res = resourcesInfo.at(i);
					res.Offset = AllocResource(i, res.Chunks,
						res.IsTemporal() ? 0 : res.StartLevel,
						res.IsTemporal() ? levelCount : res.LastLevel,
						maxChunksUAV, levelCount, invalidID, memory);
				}

#if _ZE_DEBUG_FRAME_NO_ALIASING_MEMORY
				heapDesc.SizeInBytes = Utils::SafeCast<U64>(maxChunksUAV) * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
#else
				// Check resource aliasing
				for (RID i = rt_dsCount; i < resourcesInfo.size(); ++i)
				{
					auto& res = resourcesInfo.at(i);
					if (!res.IsTemporal() && CheckResourceAliasing(res.Offset, res.Chunks, res.StartLevel,
						res.LastLevel, maxChunksUAV, levelCount, invalidID, memory))
						res.SetAliasing();
				}
				// Find final size for UAV only heap and create it with resources
				heapDesc.SizeInBytes = FindHeapSize(maxChunksUAV, levelCount, invalidID, memory);
#endif
				ZE_DX_THROW_FAILED(device->CreateHeap(&heapDesc, IID_PPV_ARGS(&uavHeap)));
				ZE_DX_THROW_FAILED(device->SetResidencyPriority(1, reinterpret_cast<IPageable**>(uavHeap.GetAddressOf()), &residencyPriority));
				heapDesc.Flags &= ~D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES;
				heapDesc.Flags |= D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;

#if _ZE_DEBUG_FRAME_MEMORY_PRINT
				PrintMemory("T1_UAV", maxChunksUAV, levelCount, invalidID, memory, heapDesc.SizeInBytes);
#endif
				for (RID i = rt_dsCount; i < resourcesInfo.size(); ++i)
				{
					auto& res = resourcesInfo.at(i);
					const auto& lifetime = desc.ResourceLifetimes.at(res.Handle);
					GFX::Resource::State firstState = lifetime.begin()->second;
					GFX::Resource::State lastState = lifetime.rbegin()->second;
					ZE_DX_THROW_FAILED(device->CreatePlacedResource(uavHeap.Get(),
						res.Offset * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
						&res.Desc, GetResourceState(_ZE_RENDERER_NO_SPLIT_BARRIERS || res.IsAliasing() ? firstState : lastState),
						res.IsRTV() || res.IsDSV() ? &res.ClearVal : nullptr, IID_PPV_ARGS(&res.Resource)));
					ZE_DX_SET_ID(res.Resource, "RID:" + std::to_string(res.Handle));
					if (lastState != firstState)
					{
						U64 lastLevel = 2 * lifetime.rbegin()->first + 1;
#if !_ZE_RENDERER_NO_SPLIT_BARRIERS
						if (res.IsAliasing())
						{
#endif
							desc.TransitionsPerLevel.at(lastLevel).emplace_back(res.Handle,
								GFX::Pipeline::BarrierType::Immediate, lastState, firstState);
#if !_ZE_RENDERER_NO_SPLIT_BARRIERS
						}
						else
						{
							desc.TransitionsPerLevel.at(lastLevel).emplace_back(res.Handle,
								GFX::Pipeline::BarrierType::Begin, lastState, firstState);
							D3D12_RESOURCE_BARRIER barrier = {};
							barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
							barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
							barrier.Transition.StateBefore = GetResourceState(lastState);
							barrier.Transition.StateAfter = GetResourceState(firstState);
							barrier.Transition.pResource = res.Resource.Get();
							barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
							startingTransitions.emplace_back(barrier);
							barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
							wrappingTransitions.emplace_back(lifetime.begin()->first, barrier);
						}
#endif
					}
				}
			}
		}
		else
		{
			// Sort resources descending by size
			rt_dsCount = Utils::SafeCast<RID>(resourcesInfo.size());
			for (const auto& res : resourcesInfo)
				maxChunks += res.Chunks;
			std::sort(resourcesInfo.begin(), resourcesInfo.end(),
				[](const auto& r1, const auto& r2) -> bool
				{
					return r1.Chunks > r2.Chunks;
				});
		}

		// Find free memory regions for resources
		memory.assign(maxChunks * levelCount, invalidID);
		for (RID i = 0; i < rt_dsCount; ++i)
		{
			auto& res = resourcesInfo.at(i);
			res.Offset = AllocResource(i, res.Chunks,
				res.IsTemporal() ? 0 : res.StartLevel,
				res.IsTemporal() ? levelCount : res.LastLevel,
				maxChunks, levelCount, invalidID, memory);
		}

#if _ZE_DEBUG_FRAME_NO_ALIASING_MEMORY
		heapDesc.SizeInBytes = Utils::SafeCast<U64>(maxChunks) * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
#else
		// Check resource aliasing
		for (RID i = 0; i < rt_dsCount; ++i)
		{
			auto& res = resourcesInfo.at(i);
			if (!res.IsTemporal() && CheckResourceAliasing(res.Offset, res.Chunks,
				res.StartLevel, res.LastLevel, maxChunks, levelCount, invalidID, memory))
				res.SetAliasing();
		}
		// Find final size for heap and create it with resources
		heapDesc.SizeInBytes = FindHeapSize(maxChunks, levelCount, invalidID, memory);
#endif
		ZE_DX_THROW_FAILED(device->CreateHeap(&heapDesc, IID_PPV_ARGS(&mainHeap)));
		ZE_DX_THROW_FAILED(device->SetResidencyPriority(1, reinterpret_cast<IPageable**>(mainHeap.GetAddressOf()), &residencyPriority));

#if _ZE_DEBUG_FRAME_MEMORY_PRINT
		PrintMemory(dev.Get().dx12.GetCurrentAllocTier() == AllocatorGPU::AllocTier::Tier1 ? "T1" : "T2", maxChunks, levelCount, invalidID, memory, heapDesc.SizeInBytes);
#endif
		for (U64 i = 0; i < rt_dsCount; ++i)
		{
			auto& res = resourcesInfo.at(i);
			const auto& lifetime = desc.ResourceLifetimes.at(res.Handle);
			GFX::Resource::State firstState = lifetime.begin()->second;
			GFX::Resource::State lastState = lifetime.rbegin()->second;
			ZE_DX_THROW_FAILED(device->CreatePlacedResource(mainHeap.Get(),
				res.Offset * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
				&res.Desc, GetResourceState(_ZE_RENDERER_NO_SPLIT_BARRIERS || res.IsAliasing() ? firstState : lastState),
				res.IsRTV() || res.IsDSV() ? &res.ClearVal : nullptr, IID_PPV_ARGS(&res.Resource)));
			ZE_DX_SET_ID(res.Resource, "RID_" + std::to_string(res.Handle));
			if (lastState != firstState)
			{
				U64 lastLevel = 2 * lifetime.rbegin()->first + 1;
#if !_ZE_RENDERER_NO_SPLIT_BARRIERS
				if (res.IsAliasing())
				{
#endif
					desc.TransitionsPerLevel.at(lastLevel).emplace_back(res.Handle,
						GFX::Pipeline::BarrierType::Immediate, lastState, firstState);
#if !_ZE_RENDERER_NO_SPLIT_BARRIERS
				}
				else
				{
					desc.TransitionsPerLevel.at(lastLevel).emplace_back(res.Handle,
						GFX::Pipeline::BarrierType::Begin, lastState, firstState);
					D3D12_RESOURCE_BARRIER barrier = {};
					barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
					barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
					barrier.Transition.StateBefore = GetResourceState(lastState);
					barrier.Transition.StateAfter = GetResourceState(firstState);
					barrier.Transition.pResource = res.Resource.Get();
					barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
					startingTransitions.emplace_back(barrier);
					barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
					wrappingTransitions.emplace_back(lifetime.begin()->first, barrier);
				}
#endif
			}
		}

#if !_ZE_RENDERER_NO_SPLIT_BARRIERS
		// Perform initial transitions for wrapping consistency
		U64 transitionFence = 0;
		if (startingTransitions.size() > 0)
		{
			mainList.Get().dx12.Open(dev);
			ZE_DRAW_TAG_BEGIN(dev, mainList.Get().dx12, "DX12 Wrapping Transitions", PixelVal::Gray);
			mainList.Get().dx12.GetList()->ResourceBarrier(Utils::SafeCast<U32>(startingTransitions.size()), startingTransitions.data());
			ZE_DRAW_TAG_END(dev, mainList.Get().dx12);
			mainList.Get().dx12.Close(dev);
			dev.Get().dx12.ExecuteMain(mainList);
			transitionFence = dev.Get().dx12.SetMainFence();
		}
		// Sort wrapping transitions in descending order
		std::sort(wrappingTransitions.begin(), wrappingTransitions.end(),
			[](const auto& t1, const auto& t2) -> bool
			{
				return t1.first > t2.first;
			});
#endif

		// Sort resources for increasing RID
		std::sort(resourcesInfo.begin(), resourcesInfo.end(),
			[](const auto& r1, const auto& r2) -> bool
			{
				return r1.Handle < r2.Handle;
			});
		aliasingResources = new bool[resourcesInfo.size()];
		resources = new BufferData[invalidID];
		resources[0].Resource = nullptr;
		resources[0].Size = desc.ResourceInfo.front().Sizes;
		resources[0].Array = desc.ResourceInfo.front().ArraySize;
		resources[0].Mips = desc.ResourceInfo.front().MipLevels;
		resources[0].Format = desc.ResourceInfo.front().Format;
		for (RID i = 1; auto & res : resourcesInfo)
		{
			aliasingResources[i - 1] = res.IsAliasing();
			auto& data = resources[i];
			data.Resource = std::move(res.Resource);
			data.Size = desc.ResourceInfo.at(i).Sizes;
			data.Array = desc.ResourceInfo.at(i).ArraySize;
			data.Mips = desc.ResourceInfo.at(i).MipLevels;
			data.Format = desc.ResourceInfo.at(i).Format;
			if (desc.ResourceInfo.at(i).Flags & GFX::Pipeline::FrameResourceFlags::Cube)
				data.SetCube();
			++i;
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

		// Get sizes of descriptors
		rtvDsv = new D3D12_CPU_DESCRIPTOR_HANDLE[invalidID];
		srv = new std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE>[invalidID];
		uav = new std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE>>[invalidID - 1];
		const U32 rtvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		const U32 dsvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		const U32 srvUavDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescHeap->GetCPUDescriptorHandleForHeapStart();
		const RID uavDescCount = uavCount + uavAdditionalMipsCount;
		descInfo = dev.Get().dx12.AllocDescs(srvUavCount + uavDescCount);
		if (uavDescCount)
			descInfoCpu = dev.Get().dx12.AllocDescs(uavDescCount, false);
		D3D12_CPU_DESCRIPTOR_HANDLE srvUavShaderVisibleHandle = descInfo.CPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvUavShaderVisibleHandleGpu = descInfo.GPU;
		D3D12_CPU_DESCRIPTOR_HANDLE uavHandle = descInfoCpu.CPU;
		// Create demanded views for each resource
		for (RID i = 1; const auto& res : resourcesInfo)
		{
			if (res.IsRTV())
			{
				D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
				rtvDesc.Format = res.Desc.Format;
				if (res.Desc.DepthOrArraySize > 1)
				{
					rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
					rtvDesc.Texture2DArray.MipSlice = 0;
					rtvDesc.Texture2DArray.FirstArraySlice = 0;
					rtvDesc.Texture2DArray.ArraySize = res.Desc.DepthOrArraySize;
					rtvDesc.Texture2DArray.PlaneSlice = 0;
					resources[i].SetArrayView();
				}
				else
				{
					rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
					rtvDesc.Texture2D.MipSlice = 0;
					rtvDesc.Texture2D.PlaneSlice = 0;
				}
				ZE_DX_THROW_FAILED_INFO(device->CreateRenderTargetView(resources[i].Resource.Get(), &rtvDesc, rtvHandle));
				rtvDsv[i] = rtvHandle;
				rtvHandle.ptr += rtvDescSize;

				// Generate views for proper mips
				if (res.Desc.MipLevels > 1)
				{
					auto& targetResourceMip = rtvDsvMips[i - 1];
					targetResourceMip = new D3D12_CPU_DESCRIPTOR_HANDLE[res.Desc.MipLevels];
					targetResourceMip[0] = rtvDsv[i];
					for (U16 j = 1; j < res.Desc.MipLevels; ++j)
					{
						if (res.Desc.DepthOrArraySize > 1)
							rtvDesc.Texture2DArray.MipSlice = j;
						else
							rtvDesc.Texture2D.MipSlice = j;

						ZE_DX_THROW_FAILED_INFO(device->CreateRenderTargetView(resources[i].Resource.Get(), &rtvDesc, rtvHandle));
						targetResourceMip[j] = rtvHandle;
						rtvHandle.ptr += rtvDescSize;
					}
				}
			}
			else if (res.IsDSV())
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
					resources[i].SetArrayView();
				}
				else
				{
					dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
					dsvDesc.Texture2D.MipSlice = 0;
				}
				ZE_DX_THROW_FAILED_INFO(device->CreateDepthStencilView(resources[i].Resource.Get(), &dsvDesc, dsvHandle));
				rtvDsv[i] = dsvHandle;
				dsvHandle.ptr += dsvDescSize;

				// Generate views for proper mips
				if (res.Desc.MipLevels > 1)
				{
					auto& targetResourceMip = rtvDsvMips[i - 1];
					targetResourceMip = new D3D12_CPU_DESCRIPTOR_HANDLE[res.Desc.MipLevels];
					targetResourceMip[0] = rtvDsv[i];
					for (U16 j = 1; j < res.Desc.MipLevels; ++j)
					{
						if (res.Desc.DepthOrArraySize > 1)
							dsvDesc.Texture2DArray.MipSlice = j;
						else
							dsvDesc.Texture2D.MipSlice = j;

						ZE_DX_THROW_FAILED_INFO(device->CreateDepthStencilView(resources[i].Resource.Get(), &dsvDesc, dsvHandle));
						targetResourceMip[j] = dsvHandle;
						dsvHandle.ptr += dsvDescSize;
					}
				}
			}
			else
				rtvDsv[i].ptr = UINT64_MAX;
			if (res.IsSRV())
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Format = DX::ConvertDepthFormatToResourceView(res.Desc.Format, res.UseStencilView());
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				if (res.IsCube())
				{
					if (res.Desc.DepthOrArraySize > 6)
					{
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
						srvDesc.TextureCubeArray.MostDetailedMip = 0;
						srvDesc.TextureCubeArray.MipLevels = res.Desc.MipLevels;
						srvDesc.TextureCubeArray.First2DArrayFace = 0;
						srvDesc.TextureCubeArray.NumCubes = res.Desc.DepthOrArraySize / 6;
						srvDesc.TextureCubeArray.ResourceMinLODClamp = 0.0f;
						resources[i].SetArrayView();
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
					resources[i].SetArrayView();
				}
				else
				{
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					srvDesc.Texture2D.MostDetailedMip = 0;
					srvDesc.Texture2D.MipLevels = res.Desc.MipLevels;
					srvDesc.Texture2D.PlaneSlice = 0;
					srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
				}
				ZE_DX_THROW_FAILED_INFO(device->CreateShaderResourceView(resources[i].Resource.Get(), &srvDesc, srvUavShaderVisibleHandle));
				srv[i] = { srvUavShaderVisibleHandle, srvUavShaderVisibleHandleGpu };
				srvUavShaderVisibleHandle.ptr += srvUavDescSize;
				srvUavShaderVisibleHandleGpu.ptr += srvUavDescSize;
			}
			else
				srv[i].first.ptr = srv[i].second.ptr = UINT64_MAX;
			++i;
		}
		// Split processing so UAV and SRV descriptors are placed next to each other
		for (RID i = 1; const auto& res : resourcesInfo)
		{
			if (res.IsUAV())
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = DX::ConvertDepthFormatToResourceView(res.Desc.Format, res.UseStencilView());
				if (res.Desc.DepthOrArraySize > 1)
				{
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
					uavDesc.Texture2DArray.MipSlice = 0;
					uavDesc.Texture2DArray.FirstArraySlice = 0;
					uavDesc.Texture2DArray.ArraySize = res.Desc.DepthOrArraySize;
					uavDesc.Texture2DArray.PlaneSlice = 0;
					resources[i].SetArrayView();
				}
				else
				{
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
					uavDesc.Texture2D.MipSlice = 0;
					uavDesc.Texture2D.PlaneSlice = 0;
				}
				ZE_DX_THROW_FAILED_INFO(device->CreateUnorderedAccessView(resources[i].Resource.Get(), nullptr, &uavDesc, uavHandle));
				device->CopyDescriptorsSimple(1, srvUavShaderVisibleHandle, uavHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				uav[i - 1] = { uavHandle, { srvUavShaderVisibleHandle, srvUavShaderVisibleHandleGpu } };
				uavHandle.ptr += srvUavDescSize;
				srvUavShaderVisibleHandle.ptr += srvUavDescSize;
				srvUavShaderVisibleHandleGpu.ptr += srvUavDescSize;

				// Generate views for proper mips
				if (res.Desc.MipLevels > 1)
				{
					auto& targetResourceMip = uavMips[i - 1];
					targetResourceMip = new std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE>>[res.Desc.MipLevels];
					targetResourceMip[0] = uav[i - 1];

					D3D12_CPU_DESCRIPTOR_HANDLE dstStart = srvUavShaderVisibleHandle;
					D3D12_CPU_DESCRIPTOR_HANDLE srcStart = uavHandle;
					for (U16 j = 1; j < res.Desc.MipLevels; ++j)
					{
						if (res.Desc.DepthOrArraySize > 1)
							uavDesc.Texture2DArray.MipSlice = j;
						else
							uavDesc.Texture2D.MipSlice = j;

						ZE_DX_THROW_FAILED_INFO(device->CreateUnorderedAccessView(resources[i].Resource.Get(), nullptr, &uavDesc, uavHandle));
						targetResourceMip[j] = { uavHandle, { srvUavShaderVisibleHandle, srvUavShaderVisibleHandleGpu } };
						uavHandle.ptr += srvUavDescSize;
						srvUavShaderVisibleHandle.ptr += srvUavDescSize;
						srvUavShaderVisibleHandleGpu.ptr += srvUavDescSize;
					}
					device->CopyDescriptorsSimple(res.Desc.MipLevels - 1, dstStart, srcStart, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				}
			}
			else
				uav[i - 1].first.ptr = uav[i - 1].second.first.ptr = uav[i - 1].second.second.ptr = UINT64_MAX;
			++i;
		}

		// Find number of barriers and prepare aux structures
		transitionSyncs = new GFX::Pipeline::SyncType[levelCount];
		transitions = new TransitionPoint[levelCount];
		U64 barrierCount = 0;
		for (const auto& level : desc.TransitionsPerLevel)
		{
			for (const auto& transition : level)
			{
#if _ZE_RENDERER_NO_SPLIT_BARRIERS
				if (transition.Barrier == GFX::Pipeline::BarrierType::End)
					continue;
				++barrierCount;
#endif
				if (transition.RID == 0)
					++backbufferBarriersLocationsCount;
			}
#if !_ZE_RENDERER_NO_SPLIT_BARRIERS
			barrierCount += level.size();
#endif
		}
		if (backbufferBarriersLocationsCount > 1)
		{
			backbufferBarriersLocations = new U64[--backbufferBarriersLocationsCount];
			backbufferBarriersLocations[0] = 0;
		}
		else
			backbufferBarriersLocationsCount = 0;
#if !_ZE_RENDERER_NO_SPLIT_BARRIERS
		barrierCount += wrappingTransitions.size();
#endif
		initTransitions.Barriers = new D3D12_RESOURCE_BARRIER[barrierCount];
		initTransitions.BarrierCount = 0;
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
#if _ZE_RENDERER_NO_SPLIT_BARRIERS
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
#endif
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		// Compute barriers before first render passes
		U64 backbufferBarrierIndex = 0;
		auto computeBarriers = [&](TransitionPoint& transitions, const U64 level, const U64 barrierIndex)
			{
				for (const auto& transition : desc.TransitionsPerLevel.at(level))
				{
#if _ZE_RENDERER_NO_SPLIT_BARRIERS
					if (transition.Barrier == GFX::Pipeline::BarrierType::End)
						continue;
#else
					barrier.Flags = GetTransitionType(transition.Barrier);
#endif
					barrier.Transition.StateBefore = GetResourceState(transition.BeforeState);
					barrier.Transition.StateAfter = GetResourceState(transition.AfterState);
					barrier.Transition.pResource = resources[transition.RID].Resource.Get();

					transitions.Barriers[transitions.BarrierCount++] = barrier;
					if (transition.RID == 0 && level != 0)
					{
						backbufferBarriersLocations[backbufferBarrierIndex++] = barrierIndex;
						if (transitions.BarrierCount != 1)
						{
							ZE_ASSERT(transitions.Barriers[0].Transition.pResource != nullptr, "Backbuffer barrier already at top of the list!");
							transitions.Barriers[transitions.BarrierCount - 1] = transitions.Barriers[0];
							transitions.Barriers[0] = barrier;
						}
					}
				}
#if !_ZE_RENDERER_NO_SPLIT_BARRIERS
				while (wrappingTransitions.size() != 0 && wrappingTransitions.back().first == barrierIndex)
				{
					transitions.Barriers[transitions.BarrierCount++] = wrappingTransitions.back().second;
					wrappingTransitions.pop_back();
				}
#endif
			};
		computeBarriers(initTransitions, 0, 0);
#if !_ZE_RENDERER_NO_SPLIT_BARRIERS
		for (auto& wrap : wrappingTransitions)
			--wrap.first;
#endif
		transitions[0].Barriers = initTransitions.Barriers + initTransitions.BarrierCount;
		transitions[0].BarrierCount = 0;
		// Compute normal barriers between passes
		for (U64 i = 1; i < desc.TransitionsPerLevel.size(); ++i)
		{
			const U64 index = i / 2 - (i % 2 == 0);
			if (index != 0 && i % 2 != 0)
			{
				transitions[index].Barriers = transitions[index - 1].Barriers + transitions[index - 1].BarrierCount;
				transitions[index].BarrierCount = 0;
			}
			computeBarriers(transitions[index], i, index);
		}

		// Check what kind of transition sync is needed
		auto computeBarrierSyncs = [](TransitionPoint& transitions, GFX::Pipeline::SyncType* syncBefore)
			{
				bool copyBefore = false, computeBefore = false;
				bool copyAfter = false, computeAfter = false;
				for (U32 i = 0; i < transitions.BarrierCount; ++i)
				{
					auto& barrier = transitions.Barriers[i];
					// TODO: Compute syncs based on passes from which resources are used
					if (syncBefore && barrier.Flags != D3D12_RESOURCE_BARRIER_FLAG_END_ONLY)
					{
						if (barrier.Transition.StateBefore & (D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
							computeBefore = true;

						if ((barrier.Transition.StateBefore & D3D12_RESOURCE_STATE_GENERIC_READ) == D3D12_RESOURCE_STATE_GENERIC_READ
							|| barrier.Transition.StateBefore == D3D12_RESOURCE_STATE_COMMON)
							copyBefore = true;
					}

					if (barrier.Flags != D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY)
					{
						if (barrier.Transition.StateBefore & (D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
							computeAfter = true;

						if ((barrier.Transition.StateBefore & D3D12_RESOURCE_STATE_GENERIC_READ) == D3D12_RESOURCE_STATE_GENERIC_READ
							|| barrier.Transition.StateBefore == D3D12_RESOURCE_STATE_COMMON)
							copyAfter = true;
					}
				}
				if (syncBefore)
				{
					if (copyBefore && computeBefore)
						*syncBefore = GFX::Pipeline::SyncType::AllToMain;
					else if (copyBefore)
						*syncBefore = GFX::Pipeline::SyncType::CopyToMain;
					else if (computeBefore)
						*syncBefore = GFX::Pipeline::SyncType::ComputeToMain;
					else
						*syncBefore = GFX::Pipeline::SyncType::None;
				}

				if (copyAfter && computeAfter)
					transitions.AfterSync = GFX::Pipeline::SyncType::MainToAll;
				else if (copyAfter)
					transitions.AfterSync = GFX::Pipeline::SyncType::MainToCopy;
				else if (computeAfter)
					transitions.AfterSync = GFX::Pipeline::SyncType::MainToCompute;
			};
		computeBarrierSyncs(initTransitions, nullptr);
		for (U64 i = 0; i < levelCount; ++i)
			computeBarrierSyncs(transitions[i], transitionSyncs + i);

#if !_ZE_RENDERER_NO_SPLIT_BARRIERS
		// Finish initial transitions
		if (startingTransitions.size() > 0)
		{
			dev.Get().dx12.WaitMain(transitionFence);
			mainList.Get().dx12.Reset(dev);
			startingTransitions.clear();
		}
#endif
	}

	FrameBuffer::~FrameBuffer()
	{
		ZE_ASSERT_FREED(descInfo.Handle == nullptr && descInfoCpu.Handle == nullptr
			&& rtvDescHeap == nullptr && dsvDescHeap == nullptr
			&& mainHeap == nullptr && uavHeap == nullptr);

		if (initTransitions.Barriers)
			initTransitions.Barriers.DeleteArray();
		if (transitionSyncs)
			transitionSyncs.DeleteArray();
		if (transitions)
			transitions.DeleteArray();
		if (backbufferBarriersLocations)
			backbufferBarriersLocations.DeleteArray();
		if (aliasingResources)
			aliasingResources.DeleteArray();
		if (resources)
			resources.DeleteArray();
		if (rtvDsv)
			rtvDsv.DeleteArray();
		if (srv)
			srv.DeleteArray();
		if (uav)
			uav.DeleteArray();
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

	void FrameBuffer::Copy(GFX::CommandList& cl, RID src, RID dest) const noexcept
	{
		ZE_ASSERT(src < resourceCount, "Source resource ID outside available range!");
		ZE_ASSERT(dest < resourceCount, "Destination resource ID outside available range!");
		ZE_ASSERT(GetDimmensions(src) == GetDimmensions(dest), "Resources must have same dimmensions for copy!");

		cl.Get().dx12.GetList()->CopyResource(resources[dest].Resource.Get(), resources[src].Resource.Get());
	}

	void FrameBuffer::SetRTV(GFX::CommandList& cl, RID rid) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rtvDsv[rid].ptr != UINT64_MAX, "Current resource is not suitable for being render target!");

		SetViewport(cl.Get().dx12, rid);
		cl.Get().dx12.GetList()->OMSetRenderTargets(1, rtvDsv + rid, true, nullptr);
	}

	void FrameBuffer::SetRTV(GFX::CommandList& cl, RID rid, U16 mipLevel) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rtvDsv[rid].ptr != UINT64_MAX, "Current resource is not suitable for being render target!");
		ZE_ASSERT(rtvDsvMips != nullptr, "Mips not supported as no resource has been created with mips greater than 1!");
		ZE_ASSERT(rtvDsvMips[rid - 1] != nullptr, "Mips for current resource not supported!");

		SetViewport(cl.Get().dx12, rid);
		cl.Get().dx12.GetList()->OMSetRenderTargets(1, rtvDsvMips[rid - 1] + mipLevel, TRUE, nullptr);
	}

	void FrameBuffer::SetDSV(GFX::CommandList& cl, RID rid) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(rtvDsv[rid].ptr != UINT64_MAX, "Current resource is not suitable for being depth stencil!");

		SetViewport(cl.Get().dx12, rid);
		cl.Get().dx12.GetList()->OMSetRenderTargets(0, nullptr, TRUE, rtvDsv + rid);
	}

	void FrameBuffer::SetDSV(GFX::CommandList& cl, RID rid, U16 mipLevel) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(rtvDsv[rid].ptr != UINT64_MAX, "Current resource is not suitable for being depth stencil!");
		ZE_ASSERT(rtvDsvMips != nullptr, "Mips not supported as no resource has been created with mips greater than 1!");
		ZE_ASSERT(rtvDsvMips[rid - 1] != nullptr, "Mips for current resource not supported!");

		SetViewport(cl.Get().dx12, rid);
		cl.Get().dx12.GetList()->OMSetRenderTargets(0, nullptr, TRUE, rtvDsvMips[rid - 1] + mipLevel);
	}

	void FrameBuffer::SetOutput(GFX::CommandList& cl, RID rtv, RID dsv) const noexcept
	{
		ZE_ASSERT(rtv < resourceCount, "RTV resource ID outside available range!");
		ZE_ASSERT(dsv < resourceCount, "DSV resource ID outside available range!");
		ZE_ASSERT(dsv != 0, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(rtvDsv[rtv].ptr != UINT64_MAX, "Current resource is not suitable for being render target!");
		ZE_ASSERT(rtvDsv[dsv].ptr != UINT64_MAX, "Current resource is not suitable for being depth stencil!");

		SetViewport(cl.Get().dx12, rtv);
		cl.Get().dx12.GetList()->OMSetRenderTargets(1, rtvDsv + rtv, TRUE, rtvDsv + dsv);
	}

	void FrameBuffer::SetRTVSparse(GFX::CommandList& cl, const RID* rid, U8 count) const noexcept
	{
		ZE_ASSERT(count <= D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT, "Too many render targets!");

		D3D12_CPU_DESCRIPTOR_HANDLE handles[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
		D3D12_VIEWPORT vieports[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
		D3D12_RECT scissorRects[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
		U8 realCount = 0;
		for (U32 i = 0; i < count; ++i)
		{
			RID id = rid[i];
			if (id != INVALID_RID)
			{
				ZE_ASSERT(id < resourceCount, "Resource ID outside available range!");

				handles[realCount] = rtvDsv[id];
				ZE_ASSERT(handles[realCount].ptr != UINT64_MAX, "Current resource is not suitable for being render target!");

				SetupViewport(vieports[realCount], scissorRects[realCount], id);
				++realCount;
			}
		}
		cl.Get().dx12.GetList()->RSSetViewports(realCount, vieports);
		cl.Get().dx12.GetList()->RSSetScissorRects(realCount, scissorRects);
		cl.Get().dx12.GetList()->OMSetRenderTargets(realCount, handles, false, nullptr);
	}

	void FrameBuffer::SetOutputSparse(GFX::CommandList& cl, const RID* rtv, RID dsv, U8 count) const noexcept
	{
		ZE_ASSERT(count <= D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT, "Too many render targets!");
		ZE_ASSERT(dsv != 0, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(rtvDsv[dsv].ptr != UINT64_MAX, "Current resource is not suitable for being depth stencil!");

		D3D12_CPU_DESCRIPTOR_HANDLE handles[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
		D3D12_VIEWPORT vieports[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
		D3D12_RECT scissorRects[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
		U8 realCount = 0;
		for (U32 i = 0; i < count; ++i)
		{
			RID id = rtv[i];
			if (id != INVALID_RID)
			{
				ZE_ASSERT(id < resourceCount, "Resource ID outside available range!");

				handles[realCount] = rtvDsv[id];
				ZE_ASSERT(handles[i].ptr != UINT64_MAX, "Current resource is not suitable for being render target!");

				SetupViewport(vieports[realCount], scissorRects[realCount], id);
				++realCount;
			}
		}
		cl.Get().dx12.GetList()->RSSetViewports(count, vieports);
		cl.Get().dx12.GetList()->RSSetScissorRects(count, scissorRects);
		cl.Get().dx12.GetList()->OMSetRenderTargets(count, handles, false, rtvDsv + dsv);
	}

	void FrameBuffer::SetSRV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(srv[rid].second.ptr != UINT64_MAX, "Current resource is not suitable for being shader resource!");

		const auto& schema = bindCtx.BindingSchema.Get().dx12;
		ZE_ASSERT(schema.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::SRV
			|| schema.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::Table,
			"Bind slot is not a shader resource or table! Wrong root signature or order of bindings!");

		auto* list = cl.Get().dx12.GetList();
		if (schema.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::SRV)
		{
			if (schema.IsCompute())
				list->SetComputeRootShaderResourceView(bindCtx.Count++, resources[rid].Resource->GetGPUVirtualAddress());
			else
				list->SetGraphicsRootShaderResourceView(bindCtx.Count++, resources[rid].Resource->GetGPUVirtualAddress());
		}
		else if (schema.IsCompute())
			list->SetComputeRootDescriptorTable(bindCtx.Count++, srv[rid].second);
		else
			list->SetGraphicsRootDescriptorTable(bindCtx.Count++, srv[rid].second);
	}

	void FrameBuffer::SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as unnordered access!");
		ZE_ASSERT(uav[rid - 1].second.second.ptr != UINT64_MAX, "Current resource is not suitable for being unnordered access!");

		const auto& schema = bindCtx.BindingSchema.Get().dx12;
		ZE_ASSERT(schema.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::UAV
			|| schema.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::Table,
			"Bind slot is not a unnordered access or table! Wrong root signature or order of bindings!");

		auto* list = cl.Get().dx12.GetList();
		if (schema.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::UAV)
		{
			if (schema.IsCompute())
				list->SetComputeRootUnorderedAccessView(bindCtx.Count++, resources[rid].Resource->GetGPUVirtualAddress());
			else
				list->SetGraphicsRootUnorderedAccessView(bindCtx.Count++, resources[rid].Resource->GetGPUVirtualAddress());
		}
		else if (schema.IsCompute())
			list->SetComputeRootDescriptorTable(bindCtx.Count++, uav[rid - 1].second.second);
		else
			list->SetGraphicsRootDescriptorTable(bindCtx.Count++, uav[rid - 1].second.second);
	}

	void FrameBuffer::SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid, U16 mipLevel) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as unnordered access!");
		ZE_ASSERT(uav[rid - 1].second.second.ptr != UINT64_MAX, "Current resource is not suitable for being unnordered access!");
		ZE_ASSERT(uavMips != nullptr, "Mips not supported as no resource has been created with mips greater than 1!");
		ZE_ASSERT(uavMips[rid - 1] != nullptr, "Mips for current resource not supported!");

		const auto& schema = bindCtx.BindingSchema.Get().dx12;
		ZE_ASSERT(schema.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::Table,
			"Bind slot is not a table! Wrong root signature or order of bindings!");

		auto* list = cl.Get().dx12.GetList();
		if (schema.IsCompute())
			list->SetComputeRootDescriptorTable(bindCtx.Count++, uavMips[rid - 1][mipLevel].second.second);
		else
			list->SetGraphicsRootDescriptorTable(bindCtx.Count++, uavMips[rid - 1][mipLevel].second.second);
	}

	void FrameBuffer::BarrierUAV(GFX::CommandList& cl, RID rid) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as unnordered access!");
		ZE_ASSERT(uav[rid - 1].first.ptr != UINT64_MAX, "Current resource is not suitable for being unnordered access!");

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.UAV.pResource = resources[rid].Resource.Get();
		cl.Get().dx12.GetList()->ResourceBarrier(1, &barrier);
	}

	void FrameBuffer::BarrierTransition(GFX::CommandList& cl, RID rid, GFX::Resource::State before, GFX::Resource::State after) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");

		ZE_ASSERT(before != GFX::Resource::StateUnorderedAccess ||
			before == GFX::Resource::StateUnorderedAccess && uav[rid - 1].first.ptr != UINT64_MAX,
			"Current resource is not suitable for being unnordered access!");

		ZE_ASSERT(before != GFX::Resource::StateRenderTarget ||
			before == GFX::Resource::StateRenderTarget && rtvDsv[rid].ptr != UINT64_MAX,
			"Current resource is not suitable for being render target!");

		ZE_ASSERT(before != GFX::Resource::StateDepthRead ||
			before == GFX::Resource::StateDepthRead && rtvDsv[rid].ptr != UINT64_MAX,
			"Current resource is not suitable for being depth stencil!");
		ZE_ASSERT(before != GFX::Resource::StateDepthWrite ||
			before == GFX::Resource::StateDepthWrite && rtvDsv[rid].ptr != UINT64_MAX,
			"Current resource is not suitable for being depth stencil!");

		ZE_ASSERT(before != GFX::Resource::StateShaderResourceAll ||
			before == GFX::Resource::StateShaderResourceAll && srv[rid].second.ptr != UINT64_MAX,
			"Current resource is not suitable for being shader resource!");
		ZE_ASSERT(before != GFX::Resource::StateShaderResourcePS ||
			before == GFX::Resource::StateShaderResourcePS && srv[rid].second.ptr != UINT64_MAX,
			"Current resource is not suitable for being shader resource!");
		ZE_ASSERT(before != GFX::Resource::StateShaderResourceNonPS ||
			before == GFX::Resource::StateShaderResourceNonPS && srv[rid].second.ptr != UINT64_MAX,
			"Current resource is not suitable for being shader resource!");

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = resources[rid].Resource.Get();
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = GetResourceState(before);
		barrier.Transition.StateAfter = GetResourceState(after);
		cl.Get().dx12.GetList()->ResourceBarrier(1, &barrier);
	}

	void FrameBuffer::ClearRTV(GFX::CommandList& cl, RID rid, const ColorF4& color) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rtvDsv[rid].ptr != UINT64_MAX, "Current resource is not suitable for being render target!");

		cl.Get().dx12.GetList()->ClearRenderTargetView(rtvDsv[rid],
			reinterpret_cast<const float*>(&color), 0, nullptr);
	}

	void FrameBuffer::ClearDSV(GFX::CommandList& cl, RID rid, float depth, U8 stencil) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(rtvDsv[rid].ptr != UINT64_MAX, "Current resource is not suitable for being depth stencil!");

		cl.Get().dx12.GetList()->ClearDepthStencilView(rtvDsv[rid],
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depth, stencil, 0, nullptr);
	}

	void FrameBuffer::ClearUAV(GFX::CommandList& cl, RID rid, const ColorF4& color) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as unnordered access!");
		ZE_ASSERT(uav[rid - 1].first.ptr != UINT64_MAX, "Current resource is not suitable for being unnordered access!");

		auto& desc = uav[rid - 1];
		cl.Get().dx12.GetList()->ClearUnorderedAccessViewFloat(desc.second.second, desc.first,
			resources[rid].Resource.Get(), reinterpret_cast<const float*>(&color), 0, nullptr);
	}

	void FrameBuffer::ClearUAV(GFX::CommandList& cl, RID rid, const Pixel colors[4]) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as unnordered access!");
		ZE_ASSERT(uav[rid - 1].first.ptr != UINT64_MAX, "Current resource is not suitable for being unnordered access!");

		auto& desc = uav[rid - 1];
		cl.Get().dx12.GetList()->ClearUnorderedAccessViewUint(desc.second.second, desc.first,
			resources[rid].Resource.Get(), reinterpret_cast<const U32*>(colors), 0, nullptr);
	}

	void FrameBuffer::SwapBackbuffer(GFX::Device& dev, GFX::SwapChain& swapChain) noexcept
	{
		auto backbufferRtvSrv = swapChain.Get().dx12.SetCurrentBackbuffer(dev, resources[0].Resource);
		rtvDsv[0] = backbufferRtvSrv.RTV;
		srv[0].first = backbufferRtvSrv.SRVCpu;
		srv[0].second = backbufferRtvSrv.SRVGpu;
		initTransitions.Barriers[0].Transition.pResource = resources[0].Resource.Get();
		for (U64 i = 0; i < backbufferBarriersLocationsCount; ++i)
			transitions[backbufferBarriersLocations[i]].Barriers[0].Transition.pResource = resources[0].Resource.Get();
	}

	void FrameBuffer::InitTransitions(GFX::Device& dev, GFX::CommandList& cl) const
	{
		// Perform wrapping barriers
		auto& device = dev.Get().dx12;
		cl.Get().dx12.Open(device);
		ZE_DRAW_TAG_BEGIN(dev, cl.Get().dx12, "Init Transitions", PixelVal::Gray);
		cl.Get().dx12.GetList()->ResourceBarrier(initTransitions.BarrierCount, initTransitions.Barriers);
		ZE_DRAW_TAG_END(dev, cl.Get().dx12);
		cl.Get().dx12.Close(device);
		device.ExecuteMain(cl);

		// Insert waits for engines that consumes addressed resources
		switch (initTransitions.AfterSync)
		{
		case GFX::Pipeline::SyncType::MainToAll:
		{
			U64 fence = device.SetMainFence();
			device.WaitCopyFromMain(fence);
			device.WaitComputeFromMain(fence);
			break;
		}
		case GFX::Pipeline::SyncType::MainToCompute:
		{
			device.WaitComputeFromMain(device.SetMainFence());
			break;
		}
		case GFX::Pipeline::SyncType::MainToCopy:
		{
			device.WaitCopyFromMain(device.SetMainFence());
			break;
		}
		default:
			break;
		}
	}

	void FrameBuffer::ExitTransitions(GFX::Device& dev, GFX::CommandList& cl, U64 level) const
	{
		// Perform normal transitions and reseting resources to initial state for aliasing
		auto& transition = transitions[level];
		if (transition.BarrierCount > 0)
		{
			auto& device = dev.Get().dx12;

			// Insert waits if barrier adresses other engines
			switch (transitionSyncs[level])
			{
			case GFX::Pipeline::SyncType::AllToMain:
				device.WaitMainFromCopy(device.SetCopyFence()); [[fallthrough]];
			case GFX::Pipeline::SyncType::ComputeToMain:
			{
				device.WaitMainFromCompute(device.SetComputeFence());
				break;
			}
			case GFX::Pipeline::SyncType::CopyToMain:
			{
				device.WaitMainFromCopy(device.SetCopyFence());
				break;
			}
			default:
				ZE_FAIL("Incorret enum value for enter sync context!"); [[fallthrough]];
			case GFX::Pipeline::SyncType::None:
				break;
			}

			cl.Get().dx12.Open(device);
			ZE_DRAW_TAG_BEGIN(dev, cl.Get().dx12, "Exit Transitions", PixelVal::Gray);
			cl.Get().dx12.GetList()->ResourceBarrier(transition.BarrierCount, transition.Barriers);
			ZE_DRAW_TAG_END(dev, cl.Get().dx12);
			cl.Get().dx12.Close(device);
			device.ExecuteMain(cl);

			// Insert waits for engines that consumes addressed resources
			switch (transition.AfterSync)
			{
			case GFX::Pipeline::SyncType::MainToAll:
			{
				U64 fence = device.SetMainFence();
				device.WaitCopyFromMain(fence);
				device.WaitComputeFromMain(fence);
				break;
			}
			case GFX::Pipeline::SyncType::MainToCompute:
			{
				device.WaitComputeFromMain(device.SetMainFence());
				break;
			}
			case GFX::Pipeline::SyncType::MainToCopy:
			{
				device.WaitCopyFromMain(device.SetMainFence());
				break;
			}
			default:
				ZE_FAIL("Incorret enum value for exit sync context!"); [[fallthrough]];
			case GFX::Pipeline::SyncType::None:
				break;
			}
		}
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