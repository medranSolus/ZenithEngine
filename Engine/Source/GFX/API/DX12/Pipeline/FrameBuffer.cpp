#include "GFX/API/DX12/Pipeline/FrameBuffer.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX12::Pipeline
{
#ifdef _ZE_DEBUG_FRAME_MEMORY_PRINT
	void FrameBuffer::PrintMemory(std::string&& memID, U32 maxChunks, U64 levelCount,
		RID invalidID, const std::vector<RID>& memory, U64 heapSize)
	{
		const U64 pixelsPerLevel = maxChunks / levelCount;
		U64 separatorPixels = pixelsPerLevel / 20;
		if (separatorPixels < 2)
			separatorPixels = 2;
		const U64 chunkPixels = pixelsPerLevel - separatorPixels;
		Surface print(levelCount * pixelsPerLevel, maxChunks);
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

#ifndef _ZE_DEBUG_FRAME_NO_ALIASING_MEMORY
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
		return static_cast<U64>(lastChunk + 1) * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
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
#ifdef _ZE_DEBUG_FRAME_NO_ALIASING_MEMORY
		// Place resources one after another
		for (; foundOffset < maxChunks; ++foundOffset)
			if (memory.at(foundOffset * levelCount) == invalidID)
				break;
		// Indicate that this memory will be occupied by single resource
		for (U32 chunk = 0; chunk < chunks; ++chunk)
			memory.at(static_cast<U64>(foundOffset + chunk) * levelCount) = id;
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
			std::fill_n(memory.begin() + static_cast<U64>(foundOffset + chunk) * levelCount + startLevel, lastLevel - startLevel + 1, id);
		return foundOffset;
	}

	U16 FrameBuffer::GetRenderLevel(U64 passLevel, GFX::Pipeline::FrameBufferDesc& desc) noexcept
	{
		for (const auto& level : desc.RenderLevels)
		{
			if (level.first.first <= passLevel && level.first.first + level.first.second > passLevel)
			{
				return level.first.first;
			}
		}
		return desc.RenderLevels.back().first.first;
	}

	D3D12_RESOURCE_BARRIER FrameBuffer::CreateResource(ResourceInfo& res, ID3D12Heap* heap, Device& dev, GFX::Pipeline::FrameBufferDesc& desc)
	{
		ZE_GFX_ENABLE_ID(dev);

		const auto& lifetime = desc.ResourceLifetimes.at(res.Handle);
		GFX::Resource::State firstState = lifetime.begin()->second.first;
		GFX::Resource::State lastState = lifetime.rbegin()->second.first;

		ZE_GFX_THROW_FAILED(dev.GetDevice()->CreatePlacedResource(heap,
			res.Offset * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
			&res.Desc, GetResourceState(res.IsAliasing() ? firstState : lastState),
			res.IsRTV() || res.IsDSV() ? &res.ClearVal : nullptr, IID_PPV_ARGS(&res.Resource)));
		ZE_GFX_SET_ID(res.Resource, "RID:" + std::to_string(res.Handle));

		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = nullptr;

		const U64 lastLevel = 2 * lifetime.rbegin()->first + 1;
		const U64 firstLevel = 2 * lifetime.begin()->first;
		if (lastState != firstState)
		{
#ifndef _ZE_DEBUG_FRAME_NO_ALIASING_MEMORY
			if (res.IsAliasing())
			{
				desc.TransitionsPerLevel.at(lastLevel).emplace_back(GFX::Pipeline::TransitionDesc(res.Handle,
					GFX::Pipeline::BarrierType::Immediate, lastState, firstState), lifetime.rbegin()->second.second);
			}
			else
#endif
			{
				desc.TransitionsPerLevel.at(lastLevel).emplace_back(GFX::Pipeline::TransitionDesc(res.Handle,
					GFX::Pipeline::BarrierType::Begin, lastState, firstState), lifetime.rbegin()->second.second);
				desc.TransitionsPerLevel.at(firstLevel).emplace_back(GFX::Pipeline::TransitionDesc(res.Handle,
					GFX::Pipeline::BarrierType::End, lastState, firstState), lifetime.begin()->second.second);

				// Cross engine barrier, need to synchronize on entry level
				if (lifetime.begin()->second.second != lifetime.rbegin()->second.second)
				{
					desc.TransitionsPerLevel.at(firstLevel).back().second = QueueType::Main;
					// Can skip synchronization and move barrier before treshold
					if (firstLevel > 0)
					{
						desc.TransitionsPerLevel.at(firstLevel - 1).emplace_back(desc.TransitionsPerLevel.at(firstLevel).back());
						desc.TransitionsPerLevel.at(firstLevel).pop_back();
					}
					else
					{
						desc.RenderLevels.at(GetRenderLevel(lifetime.begin()->first, desc)).second.first = GFX::Pipeline::SyncType::MainToAll;
					}
				}

				// Barrier to wrap resource state
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				barrier.Transition.StateBefore = GetResourceState(lastState);
				barrier.Transition.StateAfter = GetResourceState(firstState);
				barrier.Transition.pResource = res.Resource.Get();
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
			}
		}
#ifndef _ZE_DEBUG_FRAME_NO_ALIASING_MEMORY
		if (res.IsAliasing())
		{
			// Aliasing barriers
			desc.TransitionsPerLevel.at(firstLevel).emplace_back(GFX::Pipeline::TransitionDesc(res.Handle,
				GFX::Pipeline::BarrierType::Begin, GFX::Resource::StateCommon, GFX::Resource::StateCommon), QueueType::Main);
			desc.TransitionsPerLevel.at(lastLevel).emplace_back(GFX::Pipeline::TransitionDesc(res.Handle,
				GFX::Pipeline::BarrierType::End, GFX::Resource::StateCommon, GFX::Resource::StateCommon), QueueType::Main);
		}
#endif
		return barrier;
	}

	bool FrameBuffer::FillBarrier(const GFX::Pipeline::TransitionDesc& transition, D3D12_RESOURCE_BARRIER& barrier) const noexcept
	{
#ifndef _ZE_DEBUG_FRAME_NO_ALIASING_MEMORY
		if (transition.BeforeState == GFX::Resource::StateCommon
			&& transition.AfterState == GFX::Resource::StateCommon)
		{
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			if (transition.Barrier == GFX::Pipeline::BarrierType::Begin)
			{
				barrier.Aliasing.pResourceBefore = nullptr;
				barrier.Aliasing.pResourceAfter = resources[transition.RID].Resource.Get();
			}
			else
			{
				barrier.Aliasing.pResourceBefore = resources[transition.RID].Resource.Get();
				barrier.Aliasing.pResourceAfter = nullptr;
			}
		}
		else
#endif
		{
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = GetTransitionType(transition.Barrier);
			barrier.Transition.pResource = resources[transition.RID].Resource.Get();
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrier.Transition.StateBefore = GetResourceState(transition.BeforeState);
			barrier.Transition.StateAfter = GetResourceState(transition.AfterState);

			if (transition.RID == 0)
				return true;
		}
		return false;
	}

	bool FrameBuffer::FillBarrierLevelGfx(U32& barrierCount, U64 levelIndex,
		D3D12_RESOURCE_BARRIER*& barrierBuffer, GFX::Pipeline::FrameBufferDesc& desc) noexcept
	{
		bool foundCompute = false;
		for (const auto& transition : desc.TransitionsPerLevel.at(levelIndex))
		{
			if (transition.second == QueueType::Main)
			{
				if (FillBarrier(transition.first, barrierBuffer[barrierCount++]))
					backbufferBarriers[backbufferBarriersCount++] = barrierBuffer + barrierCount - 1;
			}
			else
				foundCompute = true;
		}
		barrierBuffer += barrierCount;
		return foundCompute;
	}

	void FrameBuffer::FillBarrierLevelCompute(U32& barrierCount, U64 levelIndex,
		D3D12_RESOURCE_BARRIER*& barrierBuffer, GFX::Pipeline::FrameBufferDesc& desc) noexcept
	{
		for (const auto& transition : desc.TransitionsPerLevel.at(levelIndex))
			if (transition.second == QueueType::Compute)
				FillBarrier(transition.first, barrierBuffer[barrierCount++]);
		barrierBuffer += barrierCount;
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
		scissorRect.right = size.x;
		scissorRect.bottom = size.y;
		viewport.Width = static_cast<float>(scissorRect.right);
		viewport.Height = static_cast<float>(scissorRect.bottom);

		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
	}

	void FrameBuffer::SetViewport(CommandList& cl, RID rid) const noexcept
	{
		D3D12_VIEWPORT viewport;
		D3D12_RECT scissorRect;
		SetupViewport(viewport, scissorRect, rid);
		cl.GetList()->RSSetViewports(1, &viewport);
		cl.GetList()->RSSetScissorRects(1, &scissorRect);
	}

	FrameBuffer::FrameBuffer(GFX::Graphics& gfx, GFX::Pipeline::FrameBufferDesc& desc)
	{
		auto& dev = gfx.GetDevice().Get().dx12;
		ZE_GFX_ENABLE_ID(dev);

		resourceCount = desc.ResourceInfo.size();
		ZE_ASSERT(desc.ResourceInfo.size() <= UINT16_MAX, "Too much resources, needed wider type!");

		auto* device = dev.GetDevice();
		std::vector<ResourceInfo> resourcesInfo;
		resourcesInfo.reserve(resourceCount - 1);
		U32 rtvCount = 0, rtvAdditionalMipsCount = 0;
		U32 dsvCount = 0, dsvAdditionalMipsCount = 0;
		U32 srvUavCount = 0;
		U32 uavCount = 0, uavAdditionalMipsCount = 0;
		bool rtvDsvMipsPresent = false;
		bool uavMipsPresent = false;

		// Get sizes in chunks for resources and their descriptors
		D3D12_RESOURCE_DESC resDesc;
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		resDesc.SampleDesc.Count = 1;
		resDesc.SampleDesc.Quality = 0;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		for (RID i = 1; i < resourceCount; ++i)
		{
			const auto& res = desc.ResourceInfo.at(i);
			resDesc.Width = res.Width;
			resDesc.Height = res.Height;
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

			const auto& lifetime = desc.ResourceLifetimes.at(i);
			for (const auto& state : lifetime)
			{
				switch (state.second.first)
				{
				case GFX::Resource::StateRenderTarget:
				{
					ZE_ASSERT(!isDS, "Cannot create depth stencil and render target view for same buffer!");

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

			resDesc.Format = DX::GetDXFormat(res.Format);
			D3D12_CLEAR_VALUE clearDesc;
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
			resourcesInfo.emplace_back(i, 0, static_cast<U32>(size), lifetime.begin()->first,
				lifetime.rbegin()->first, resDesc, clearDesc, nullptr,
				static_cast<U8>(isRT) | (isDS << 1) | (isSR << 2) | (isUA << 3)
				| ((res.Flags & GFX::Pipeline::FrameResourceFlags::Cube) << 4));

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
			uavMips = new Ptr<std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE>>[resourceCount - 1];
		const U64 levelCount = desc.TransitionsPerLevel.size() / 2;
		const U64 invalidID = desc.ResourceInfo.size();
		const D3D12_RESIDENCY_PRIORITY residencyPriority = D3D12_RESIDENCY_PRIORITY_MAXIMUM;
		U64 rt_dsCount;
		U32 maxChunks = 0;
		std::vector<RID> memory;
		D3D12_HEAP_DESC heapDesc;
		heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapDesc.Properties.CreationNodeMask = 0;
		heapDesc.Properties.VisibleNodeMask = 0;
		heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		heapDesc.Flags = static_cast<D3D12_HEAP_FLAGS>(D3D12_HEAP_FLAG_CREATE_NOT_ZEROED
			| D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES);
		std::vector<D3D12_RESOURCE_BARRIER> wrappingTransitions;

		// Handle resource types (Non RT/DS) depending on present tier level
		if (dev.GetCurrentAllocTier() == Device::AllocTier::Tier1)
		{
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
					res.Offset = AllocResource(i, res.Chunks, res.StartLevel, res.LastLevel, maxChunksUAV, levelCount, invalidID, memory);
				}

#ifdef _ZE_DEBUG_FRAME_NO_ALIASING_MEMORY
				heapDesc.SizeInBytes = static_cast<U64>(maxChunksUAV) * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
#else
				// Check resource aliasing
				for (RID i = rt_dsCount; i < resourcesInfo.size(); ++i)
				{
					auto& res = resourcesInfo.at(i);
					if (CheckResourceAliasing(res.Offset, res.Chunks, res.StartLevel,
						res.LastLevel, maxChunksUAV, levelCount, invalidID, memory))
						res.SetAliasing();
				}
				// Find final size for UAV only heap and create it with resources
				heapDesc.SizeInBytes = FindHeapSize(maxChunksUAV, levelCount, invalidID, memory);
#endif
				heapDesc.Flags = static_cast<D3D12_HEAP_FLAGS>(D3D12_HEAP_FLAG_CREATE_NOT_ZEROED
					| D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES);
				ZE_GFX_THROW_FAILED(device->CreateHeap(&heapDesc, IID_PPV_ARGS(&uavHeap)));
				ZE_GFX_THROW_FAILED(device->SetResidencyPriority(1, reinterpret_cast<ID3D12Pageable**>(uavHeap.GetAddressOf()), &residencyPriority));

#ifdef _ZE_DEBUG_FRAME_MEMORY_PRINT
				PrintMemory("T1_UAV", maxChunksUAV, levelCount, invalidID, memory, heapDesc.SizeInBytes);
#endif
				for (RID i = rt_dsCount; i < resourcesInfo.size(); ++i)
				{
					D3D12_RESOURCE_BARRIER wrappingBarrier = CreateResource(resourcesInfo.at(i), uavHeap.Get(), dev, desc);
					if (wrappingBarrier.Transition.pResource)
						wrappingTransitions.emplace_back(wrappingBarrier);
				}
			}
		}
		else
		{
			// Sort resources descending by size
			rt_dsCount = resourcesInfo.size();
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
			res.Offset = AllocResource(i, res.Chunks, res.StartLevel, res.LastLevel, maxChunks, levelCount, invalidID, memory);
		}

#ifdef _ZE_DEBUG_FRAME_NO_ALIASING_MEMORY
		heapDesc.SizeInBytes = static_cast<U64>(maxChunks) * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
#else
		// Check resource aliasing
		for (RID i = 0; i < rt_dsCount; ++i)
		{
			auto& res = resourcesInfo.at(i);
			if (CheckResourceAliasing(res.Offset, res.Chunks,
				res.StartLevel, res.LastLevel, maxChunks, levelCount, invalidID, memory))
				res.SetAliasing();
		}
		// Find final size for heap and create it with resources
		heapDesc.SizeInBytes = FindHeapSize(maxChunks, levelCount, invalidID, memory);
#endif
		heapDesc.Flags = static_cast<D3D12_HEAP_FLAGS>(D3D12_HEAP_FLAG_CREATE_NOT_ZEROED
			| D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES);
		ZE_GFX_THROW_FAILED(device->CreateHeap(&heapDesc, IID_PPV_ARGS(&mainHeap)));
		ZE_GFX_THROW_FAILED(device->SetResidencyPriority(1, reinterpret_cast<ID3D12Pageable**>(mainHeap.GetAddressOf()), &residencyPriority));

#ifdef _ZE_DEBUG_FRAME_MEMORY_PRINT
		PrintMemory(dev.GetCurrentAllocTier() == Device::AllocTier::Tier1 ? "T1" : "T2", maxChunks, levelCount, invalidID, memory, heapDesc.SizeInBytes);
#endif
		for (U64 i = 0; i < rt_dsCount; ++i)
		{
			D3D12_RESOURCE_BARRIER wrappingBarrier = CreateResource(resourcesInfo.at(i), mainHeap.Get(), dev, desc);
			if (wrappingBarrier.Transition.pResource)
				wrappingTransitions.emplace_back(wrappingBarrier);
		}

		// Perform initial transitions for wrapping consistency
		auto& mainList = gfx.GetMainList().Get().dx12;
		U64 transitionFence = 0;
		if (wrappingTransitions.size() > 0)
		{
			mainList.Open(dev);
			ZE_DRAW_TAG_BEGIN(mainList, L"DX12 Wrapping Transitions", PixelVal::Gray);
			mainList.GetList()->ResourceBarrier(static_cast<U32>(wrappingTransitions.size()), wrappingTransitions.data());
			ZE_DRAW_TAG_END(mainList);
			mainList.Close(dev);
			dev.ExecuteMain(gfx.GetMainList());

			transitionFence = dev.SetMainFence();
			wrappingTransitions.clear();
		}

		// Sort resources for increasing RID
		std::sort(resourcesInfo.begin(), resourcesInfo.end(),
			[](const auto& r1, const auto& r2) -> bool
			{
				return r1.Handle < r2.Handle;
			});
		aliasingResources = new bool[resourcesInfo.size()];
		resources = new BufferData[invalidID];
		resources[0].Resource = nullptr;
		resources[0].Size.x = desc.ResourceInfo.at(0).Width;
		resources[0].Size.y = desc.ResourceInfo.at(0).Height;
		for (RID i = 1; auto& res : resourcesInfo)
		{
			aliasingResources[i - 1] = res.IsAliasing();
			auto& data = resources[i];
			data.Resource = std::move(res.Resource);
			data.Size.x = desc.ResourceInfo.at(i).Width;
			data.Size.y = desc.ResourceInfo.at(i).Height;
			++i;
		}

		// Create descriptor heaps
		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc;
		descHeapDesc.NodeMask = 0;
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descHeapDesc.NumDescriptors = rtvCount + rtvAdditionalMipsCount;
		ZE_GFX_THROW_FAILED(device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&rtvDescHeap)));
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		descHeapDesc.NumDescriptors = dsvCount + dsvAdditionalMipsCount;
		ZE_GFX_THROW_FAILED(device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&dsvDescHeap)));
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descHeapDesc.NumDescriptors = uavCount + uavAdditionalMipsCount;
		ZE_GFX_THROW_FAILED(device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&uavDescHeap)));

		// Get sizes of descriptors
		rtvDsv = new D3D12_CPU_DESCRIPTOR_HANDLE[invalidID];
		srv = new D3D12_GPU_DESCRIPTOR_HANDLE[invalidID];
		uav = new std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE>[invalidID - 1];
		const U32 rtvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		const U32 dsvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		const U32 srvUavDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescHeap->GetCPUDescriptorHandleForHeapStart();
		D3D12_CPU_DESCRIPTOR_HANDLE uavHandle = uavDescHeap->GetCPUDescriptorHandleForHeapStart();
		auto srvUavHandle = dev.AddStaticDescs(srvUavCount + uavCount + uavAdditionalMipsCount);
		// Create demanded views for each resource
		for (RID i = 1; const auto& res : resourcesInfo)
		{
			if (res.IsRTV())
			{
				D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
				rtvDesc.Format = res.Desc.Format;
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
				ZE_GFX_THROW_FAILED_INFO(device->CreateRenderTargetView(resources[i].Resource.Get(), &rtvDesc, rtvHandle));
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

						ZE_GFX_THROW_FAILED_INFO(device->CreateRenderTargetView(resources[i].Resource.Get(), &rtvDesc, rtvHandle));
						targetResourceMip[j] = rtvHandle;
						rtvHandle.ptr += rtvDescSize;
					}
				}
			}
			else if (res.IsDSV())
			{
				D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
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
				ZE_GFX_THROW_FAILED_INFO(device->CreateDepthStencilView(resources[i].Resource.Get(), &dsvDesc, dsvHandle));
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

						ZE_GFX_THROW_FAILED_INFO(device->CreateDepthStencilView(resources[i].Resource.Get(), &dsvDesc, dsvHandle));
						targetResourceMip[j] = dsvHandle;
						dsvHandle.ptr += dsvDescSize;
					}
				}
			}
			else
				rtvDsv[i].ptr = UINT64_MAX;
			if (res.IsUAV())
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
				uavDesc.Format = DX::ConvertFromDepthStencilFormat(res.Desc.Format);
				if (res.Desc.DepthOrArraySize > 1)
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
				ZE_GFX_THROW_FAILED_INFO(device->CreateUnorderedAccessView(resources[i].Resource.Get(), nullptr, &uavDesc, uavHandle));
				device->CopyDescriptorsSimple(1, srvUavHandle.first, uavHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				uav[i - 1] = { uavHandle, srvUavHandle.second };
				uavHandle.ptr += srvUavDescSize;
				srvUavHandle.first.ptr += srvUavDescSize;
				srvUavHandle.second.ptr += srvUavDescSize;

				// Generate views for proper mips
				if (res.Desc.MipLevels > 1)
				{
					auto& targetResourceMip = uavMips[i - 1];
					targetResourceMip = new std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE>[res.Desc.MipLevels];
					targetResourceMip[0] = uav[i - 1];

					D3D12_CPU_DESCRIPTOR_HANDLE dstStart = srvUavHandle.first;
					D3D12_CPU_DESCRIPTOR_HANDLE srcStart = uavHandle;
					for (U16 j = 1; j < res.Desc.MipLevels; ++j)
					{
						if (res.Desc.DepthOrArraySize > 1)
							uavDesc.Texture2DArray.MipSlice = j;
						else
							uavDesc.Texture2D.MipSlice = j;

						ZE_GFX_THROW_FAILED_INFO(device->CreateUnorderedAccessView(resources[i].Resource.Get(), nullptr, &uavDesc, uavHandle));
						targetResourceMip[j] = { uavHandle, srvUavHandle.second };
						uavHandle.ptr += srvUavDescSize;
						srvUavHandle.first.ptr += srvUavDescSize;
						srvUavHandle.second.ptr += srvUavDescSize;
					}
					device->CopyDescriptorsSimple(res.Desc.MipLevels - 1, dstStart, srcStart, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				}
			}
			else
				uav[i - 1].first.ptr = uav[i - 1].second.ptr = UINT64_MAX;
			if (res.IsSRV())
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
				srvDesc.Format = DX::ConvertFromDepthStencilFormat(res.Desc.Format);
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
				ZE_GFX_THROW_FAILED_INFO(device->CreateShaderResourceView(resources[i].Resource.Get(), &srvDesc, srvUavHandle.first));
				srv[i] = srvUavHandle.second;
				srvUavHandle.first.ptr += srvUavDescSize;
				srvUavHandle.second.ptr += srvUavDescSize;
			}
			else
				srv[i].ptr = UINT64_MAX;
			++i;
		}

		// Prepare buffer for all the barriers
		U64 barrierCount = 0;
		for (const auto& passLevel : desc.TransitionsPerLevel)
		{
			for (const auto& transition : passLevel)
				if (transition.first.RID == 0)
					++backbufferBarriersCount;
			barrierCount += passLevel.size();
		}
		D3D12_RESOURCE_BARRIER* barriers = new D3D12_RESOURCE_BARRIER[barrierCount];
		backbufferBarriers = new D3D12_RESOURCE_BARRIER*[backbufferBarriersCount];
		backbufferBarriersCount = 0;

		// Compute barriers for every level and group them into:
		// single entry group before render level and exit group after every pass level
		transitionLevels = desc.RenderLevels.size();
		gfxTransitions = new LevelTransition[transitionLevels];
		computeTransitions = new LevelTransition[transitionLevels];
		for (U16 i = 0; const auto& level : desc.RenderLevels)
		{
			auto& gfx = gfxTransitions[i];
			auto& compute = computeTransitions[i];

			// Prepare entry barriers for level
			U64 levelIndex = level.first.first * 2;
			gfx.EntryBarriers = barriers;
			if (level.second.first == GFX::Pipeline::SyncType::MainToAll)
			{
				if (!syncedEntryTransitions)
				{
					syncedEntryTransitions = new std::pair<Ptr<D3D12_RESOURCE_BARRIER>, U32>[transitionLevels];
					for (U16 j = transitionLevels; j;)
						syncedEntryTransitions[--j] = { nullptr, 0 };
				}
				syncedEntryTransitions[i].first = barriers;
				for (const auto& transition : desc.TransitionsPerLevel.at(levelIndex))
				{
					if (FillBarrier(transition.first, barriers[syncedEntryTransitions[i].second++]))
						backbufferBarriers[backbufferBarriersCount++] = barriers + syncedEntryTransitions[i].second - 1;
				}
				barriers += syncedEntryTransitions[i].second;
			}
			else if (i == 0 || desc.RenderLevels.at(i - 1).second.second != GFX::Pipeline::SyncType::ComputeToAll)
			{
				if (FillBarrierLevelGfx(gfx.BarrierCount, levelIndex, barriers, desc))
				{
					compute.EntryBarriers = barriers;
					FillBarrierLevelCompute(compute.BarrierCount, levelIndex, barriers, desc);
				}
			}
			++levelIndex;

			// Fill normal barriers for each pass level
			gfx.ExitBarriers = new std::pair<Ptr<D3D12_RESOURCE_BARRIER>, U32>[level.first.second];
			compute.ExitBarriers = new std::pair<Ptr<D3D12_RESOURCE_BARRIER>, U32>[level.first.second];
			const U16 lastLevel = level.first.second - 1;
			for (U16 j = 0; j < lastLevel; ++j)
			{
				// Merge previous pass End and next one Begin barrier groups
				gfx.ExitBarriers[j].second = 0;
				gfx.ExitBarriers[j].first = barriers;
				bool computeEnd = FillBarrierLevelGfx(gfx.ExitBarriers[j].second, levelIndex, barriers, desc);
				U32 countBegin = 0;
				bool computeBegin = FillBarrierLevelGfx(countBegin, levelIndex + 1, barriers, desc);
				gfx.ExitBarriers[j].second += countBegin;

				// Same for compute
				compute.ExitBarriers[j].second = 0;
				compute.ExitBarriers[j].first = barriers;
				if (computeEnd)
					FillBarrierLevelCompute(compute.ExitBarriers[j].second, levelIndex, barriers, desc);
				if (computeBegin)
				{
					countBegin = 0;
					FillBarrierLevelCompute(countBegin, levelIndex + 1, barriers, desc);
					compute.ExitBarriers[j].second += countBegin;
				}
				levelIndex += 2;
			}

			// Handle last not full level and move correct barriers into separate group if sync needed
			if (level.second.second == GFX::Pipeline::SyncType::ComputeToAll)
			{
				if (!syncedExitTransitions)
				{
					syncedExitTransitions = new std::pair<Ptr<D3D12_RESOURCE_BARRIER>, U32>[transitionLevels - 1];
					for (U16 j = transitionLevels - 1; j;)
						syncedExitTransitions[--j] = { nullptr, 0 };
				}
				syncedExitTransitions[i].first = barriers;
				for (const auto& transition : desc.TransitionsPerLevel.at(levelIndex))
				{
					if (FillBarrier(transition.first, barriers[syncedExitTransitions[i].second++]))
						backbufferBarriers[backbufferBarriersCount++] = barriers + syncedExitTransitions[i].second - 1;
				}
				for (const auto& transition : desc.TransitionsPerLevel.at(levelIndex + 1))
				{
					if (FillBarrier(transition.first, barriers[syncedExitTransitions[i].second++]))
						backbufferBarriers[backbufferBarriersCount++] = barriers + syncedExitTransitions[i].second - 1;
				}
				barriers += syncedExitTransitions[i].second;
			}
			else
			{
				gfx.ExitBarriers[lastLevel].first = barriers;
				if (FillBarrierLevelGfx(gfx.ExitBarriers[lastLevel].second, levelIndex, barriers, desc))
				{
					compute.ExitBarriers[lastLevel].first = barriers;
					FillBarrierLevelCompute(compute.ExitBarriers[lastLevel].second, levelIndex, barriers, desc);
				}
			}
			++i;
		}

		// Finish initial transitions
		if (transitionFence > 0)
		{
			dev.WaitMain(transitionFence);
			mainList.Reset(dev);
		}
	}

	FrameBuffer::~FrameBuffer()
	{
		if (gfxTransitions)
		{
			if (gfxTransitions[0].EntryBarriers)
				gfxTransitions[0].EntryBarriers.DeleteArray();
			for (U16 i = 0; i < transitionLevels; ++i)
				gfxTransitions[i].ExitBarriers.DeleteArray();
			gfxTransitions.DeleteArray();
		}
		if (computeTransitions)
		{
			for (U16 i = 0; i < transitionLevels; ++i)
				computeTransitions[i].ExitBarriers.DeleteArray();
			computeTransitions.DeleteArray();
		}
		if (syncedEntryTransitions)
			syncedEntryTransitions.DeleteArray();
		if (syncedExitTransitions)
			syncedExitTransitions.DeleteArray();
		if (backbufferBarriers)
			backbufferBarriers.DeleteArray();
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
		ZE_ASSERT(rtvDsv[rid].ptr != -1, "Current resource is not suitable for being render target!");

		SetViewport(cl.Get().dx12, rid);
		cl.Get().dx12.GetList()->OMSetRenderTargets(1, rtvDsv + rid, TRUE, nullptr);
	}

	void FrameBuffer::SetRTV(GFX::CommandList& cl, RID rid, U16 mipLevel) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rtvDsv[rid].ptr != -1, "Current resource is not suitable for being render target!");
		ZE_ASSERT(rtvDsvMips != nullptr, "Mips not supported as no resource has been created with mips greater than 1!");
		ZE_ASSERT(rtvDsvMips[rid - 1] != nullptr, "Mips for current resource not supported!");

		SetViewport(cl.Get().dx12, rid);
		cl.Get().dx12.GetList()->OMSetRenderTargets(1, rtvDsvMips[rid - 1] + mipLevel, TRUE, nullptr);
	}

	void FrameBuffer::SetDSV(GFX::CommandList& cl, RID rid) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(rtvDsv[rid].ptr != -1, "Current resource is not suitable for being depth stencil!");

		SetViewport(cl.Get().dx12, rid);
		cl.Get().dx12.GetList()->OMSetRenderTargets(0, nullptr, TRUE, rtvDsv + rid);
	}

	void FrameBuffer::SetDSV(GFX::CommandList& cl, RID rid, U16 mipLevel) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(rtvDsv[rid].ptr != -1, "Current resource is not suitable for being depth stencil!");
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
		ZE_ASSERT(rtvDsv[rtv].ptr != -1, "Current resource is not suitable for being render target!");
		ZE_ASSERT(rtvDsv[dsv].ptr != -1, "Current resource is not suitable for being depth stencil!");

		SetViewport(cl.Get().dx12, rtv);
		cl.Get().dx12.GetList()->OMSetRenderTargets(1, rtvDsv + rtv, TRUE, rtvDsv + dsv);
	}

	void FrameBuffer::SetSRV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(srv[rid].ptr != -1, "Current resource is not suitable for being shader resource!");

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
			list->SetComputeRootDescriptorTable(bindCtx.Count++, srv[rid]);
		else
			list->SetGraphicsRootDescriptorTable(bindCtx.Count++, srv[rid]);
	}

	void FrameBuffer::SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as unnordered access!");
		ZE_ASSERT(uav[rid - 1].second.ptr != -1, "Current resource is not suitable for being unnordered access!");

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
			list->SetComputeRootDescriptorTable(bindCtx.Count++, uav[rid - 1].second);
		else
			list->SetGraphicsRootDescriptorTable(bindCtx.Count++, uav[rid - 1].second);
	}

	void FrameBuffer::SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid, U16 mipLevel) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as unnordered access!");
		ZE_ASSERT(uav[rid - 1].second.ptr != -1, "Current resource is not suitable for being unnordered access!");
		ZE_ASSERT(uavMips != nullptr, "Mips not supported as no resource has been created with mips greater than 1!");
		ZE_ASSERT(uavMips[rid - 1] != nullptr, "Mips for current resource not supported!");

		const auto& schema = bindCtx.BindingSchema.Get().dx12;
		ZE_ASSERT(schema.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::Table,
			"Bind slot is not a table! Wrong root signature or order of bindings!");

		auto* list = cl.Get().dx12.GetList();
		if (schema.IsCompute())
			list->SetComputeRootDescriptorTable(bindCtx.Count++, uavMips[rid - 1][mipLevel].second);
		else
			list->SetGraphicsRootDescriptorTable(bindCtx.Count++, uavMips[rid - 1][mipLevel].second);
	}

	void FrameBuffer::BarrierUAV(GFX::CommandList& cl, RID rid) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as unnordered access!");
		ZE_ASSERT(uav[rid - 1].first.ptr != -1, "Current resource is not suitable for being unnordered access!");

		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.UAV.pResource = resources[rid].Resource.Get();
		cl.Get().dx12.GetList()->ResourceBarrier(1, &barrier);
	}

	void FrameBuffer::BarrierTransition(GFX::CommandList& cl, RID rid, GFX::Resource::State before, GFX::Resource::State after) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");

		ZE_ASSERT(before != GFX::Resource::StateUnorderedAccess ||
			before == GFX::Resource::StateUnorderedAccess && uav[rid - 1].first.ptr != -1,
			"Current resource is not suitable for being unnordered access!");

		ZE_ASSERT(before != GFX::Resource::StateRenderTarget ||
			before == GFX::Resource::StateRenderTarget && rtvDsv[rid].ptr != -1,
			"Current resource is not suitable for being render target!");

		ZE_ASSERT(before != GFX::Resource::StateDepthRead ||
			before == GFX::Resource::StateDepthRead && rtvDsv[rid].ptr != -1,
			"Current resource is not suitable for being depth stencil!");
		ZE_ASSERT(before != GFX::Resource::StateDepthWrite ||
			before == GFX::Resource::StateDepthWrite && rtvDsv[rid].ptr != -1,
			"Current resource is not suitable for being depth stencil!");

		ZE_ASSERT(before != GFX::Resource::StateShaderResourceAll ||
			before == GFX::Resource::StateShaderResourceAll && srv[rid].ptr != -1,
			"Current resource is not suitable for being shader resource!");
		ZE_ASSERT(before != GFX::Resource::StateShaderResourcePS ||
			before == GFX::Resource::StateShaderResourcePS && srv[rid].ptr != -1,
			"Current resource is not suitable for being shader resource!");
		ZE_ASSERT(before != GFX::Resource::StateShaderResourceNonPS ||
			before == GFX::Resource::StateShaderResourceNonPS && srv[rid].ptr != -1,
			"Current resource is not suitable for being shader resource!");

		D3D12_RESOURCE_BARRIER barrier;
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
		ZE_ASSERT(rtvDsv[rid].ptr != -1, "Current resource is not suitable for being render target!");

		cl.Get().dx12.GetList()->ClearRenderTargetView(rtvDsv[rid],
			reinterpret_cast<const float*>(&color), 0, nullptr);
	}

	void FrameBuffer::ClearDSV(GFX::CommandList& cl, RID rid, float depth, U8 stencil) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(rtvDsv[rid].ptr != -1, "Current resource is not suitable for being depth stencil!");

		cl.Get().dx12.GetList()->ClearDepthStencilView(rtvDsv[rid],
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depth, stencil, 0, nullptr);
	}

	void FrameBuffer::ClearUAV(GFX::CommandList& cl, RID rid, const ColorF4& color) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as unnordered access!");
		ZE_ASSERT(uav[rid - 1].first.ptr != -1, "Current resource is not suitable for being unnordered access!");

		auto& desc = uav[rid - 1];
		cl.Get().dx12.GetList()->ClearUnorderedAccessViewFloat(desc.second, desc.first,
			resources[rid].Resource.Get(), reinterpret_cast<const float*>(&color), 0, nullptr);
	}

	void FrameBuffer::ClearUAV(GFX::CommandList& cl, RID rid, const Pixel colors[4]) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as unnordered access!");
		ZE_ASSERT(uav[rid - 1].first.ptr != -1, "Current resource is not suitable for being unnordered access!");

		auto& desc = uav[rid - 1];
		cl.Get().dx12.GetList()->ClearUnorderedAccessViewUint(desc.second, desc.first,
			resources[rid].Resource.Get(), reinterpret_cast<const U32*>(colors), 0, nullptr);
	}

	void FrameBuffer::SwapBackbuffer(GFX::Device& dev, GFX::SwapChain& swapChain) noexcept
	{
		auto backbufferRtvSrv = swapChain.Get().dx12.SetCurrentBackbuffer(dev, resources[0].Resource);
		rtvDsv[0] = backbufferRtvSrv.first;
		srv[0] = backbufferRtvSrv.second;
		for (U32 i = 0; i < backbufferBarriersCount; ++i)
			backbufferBarriers[i]->Transition.pResource = resources[0].Resource.Get();
	}

	void FrameBuffer::SyncedEntryTransitions(GFX::CommandList& cl, U16 renderLevel) const
	{
		auto& transition = syncedEntryTransitions[renderLevel];
		ZE_ASSERT(transition.second > 0, "No transition barriers at this level! Missing barriers or wrong sync type chosen.");

		cl.Get().dx12.GetList()->ResourceBarrier(transition.second, transition.first);
	}

	void FrameBuffer::SyncedExitTransitions(GFX::CommandList& cl, U16 renderLevel) const
	{
		auto& transition = syncedExitTransitions[renderLevel];
		ZE_ASSERT(transition.second > 0, "No transition barriers at this level! Missing barriers or wrong sync type chosen.");

		cl.Get().dx12.GetList()->ResourceBarrier(transition.second, transition.first);
	}

	void FrameBuffer::EntryTransitions(GFX::CommandList& cl, QueueType queue, U16 renderLevel) const
	{
		ZE_ASSERT(queue != QueueType::Copy, "Copy queue is not supported as part of render graph and target for framebuffer transitions!");

		// Perform cross-engine barriers
		auto& transition = queue == QueueType::Main ?
			gfxTransitions[renderLevel] : computeTransitions[renderLevel];

		if (transition.BarrierCount > 0)
		{
			ZE_DRAW_TAG_BEGIN(cl.Get().dx12, L"Entry Transitions", PixelVal::Gray);
			cl.Get().dx12.GetList()->ResourceBarrier(transition.BarrierCount, transition.EntryBarriers);
			ZE_DRAW_TAG_END(cl.Get().dx12);
		}
	}

	void FrameBuffer::ExitTransitions(GFX::CommandList& cl, QueueType queue, U16 renderLevel, U16 passlevel) const
	{
		ZE_ASSERT(queue != QueueType::Copy, "Copy queue is not supported as part of render graph and target for framebuffer transitions!");

		// Perform normal transitions, aliasing barriers and reseting resources to initial state for aliasing
		auto& transition = queue == QueueType::Main ?
			gfxTransitions[renderLevel].ExitBarriers[passlevel] :
			computeTransitions[renderLevel].ExitBarriers[passlevel];

		if (transition.second > 0)
		{
			ZE_DRAW_TAG_BEGIN(cl.Get().dx12, L"Exit Transitions", PixelVal::Gray);
			cl.Get().dx12.GetList()->ResourceBarrier(transition.second, transition.first);
			ZE_DRAW_TAG_END(cl.Get().dx12);
		}
	}
}