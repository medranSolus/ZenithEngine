#include "GFX/API/DX12/Pipeline/FrameBuffer.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX12::Pipeline
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
		DX::ComPtr<ID3D12Resource> Resource;
		std::bitset<6> Flags;

		constexpr bool IsRTV() const noexcept { return Flags[0]; }
		constexpr bool IsDSV() const noexcept { return Flags[1]; }
		constexpr bool IsSRV() const noexcept { return Flags[2]; }
		constexpr bool IsUAV() const noexcept { return Flags[3]; }
		constexpr bool IsCube() const noexcept { return Flags[4]; }
		constexpr bool IsAliasing() const noexcept { return Flags[5]; }
		void SetAliasing() noexcept { Flags[5] = true; }
	};

#ifdef _ZE_DEBUG_FRAME_MEMORY_PRINT
	void FrameBuffer::PrintMemory(std::string&& memID, U32 maxChunks, U64 levelCount,
		RID invalidID, const std::vector<RID>& memory, U64 heapSize)
	{
		// Otherwise wider format should be used
		assert(invalidID <= UINT32_MAX);
		U64 pixelsPerLevel = maxChunks / levelCount;
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
				for (U64 p = 0; p < pixelsPerLevel; ++p)
					print.PutPixel(level * pixelsPerLevel + p, chunk, pixel);
			}
		}
		print.Save("memory_print_dx12_" + memID + "_" + std::to_string(heapSize) + "bytes.png");
	}
#endif

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

	U32 FrameBuffer::AllocResource(U64 id, U32 chunks, U64 startLevel, U64 lastLevel,
		U32 maxChunks, U64 levelCount, RID invalidID, std::vector<RID>& memory)
	{
		U32 foundOffset = 0;
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
		// Should never happen, bug in memory table creation!!!
		if (foundOffset == maxChunks)
			throw ZE_RGC_EXCEPT("Memory too small to fit requested resource! ID: [" + std::to_string(id) +
				"] Chunks: [" + std::to_string(chunks) + "] Start level: [" + std::to_string(startLevel) +
				"] Last level: [" + std::to_string(lastLevel) + "]");
		// Reserve space in memory
		for (U32 chunk = 0; chunk < chunks; ++chunk)
			std::fill_n(memory.begin() + (foundOffset + chunk) * levelCount + startLevel, lastLevel - startLevel + 1, id);
		return foundOffset;
	}

	void FrameBuffer::InitResource(CommandList& cl, RID rid) const noexcept
	{
		// Perform discard operations for aliasing resources
		assert(rid != 0 && "Backbuffer do not need discarding it's contents! (Or at least it shouldn't with FLIP_DISCARD... TODO: Check this)");
		if (aliasingResources[--rid])
			cl.GetList()->DiscardResource(resources[rid].Get(), nullptr);
	}

	FrameBuffer::FrameBuffer(GFX::Device& dev, GFX::CommandList& mainList, GFX::Pipeline::FrameBufferDesc& desc)
	{
		ZE_GFX_ENABLE_ID(dev.Get().dx12);

		auto device = dev.Get().dx12.GetDevice();
		std::vector<ResourceInfo> resourcesInfo;
		resourcesInfo.reserve(desc.ResourceInfo.size() - 1);
		U32 rtvCount = 0;
		U32 dsvCount = 0;
		U32 srvUavCount = 0;

		// Get sizes in chunks for resources and their descriptors
		D3D12_RESOURCE_DESC resDesc;
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		resDesc.MipLevels = 1;
		resDesc.SampleDesc.Count = 1;
		resDesc.SampleDesc.Quality = 0;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		for (U64 i = 1; i < desc.ResourceInfo.size(); ++i)
		{
			const auto& res = desc.ResourceInfo.at(i);
			resDesc.Width = res.Width;
			resDesc.Height = res.Height;
			resDesc.DepthOrArraySize = res.ArraySize;
			resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			bool isRT = false, isDS = false, isUA = false, isSR = res.Flags & GFX::Pipeline::FrameResourceFlags::ForceSRV;
			const auto& lifetime = desc.ResourceLifetimes.at(i);
			for (const auto& state : lifetime)
			{
				switch (state.second)
				{
				case GFX::Resource::State::RenderTarget:
				{
					resDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
					isRT = true;
					break;
				}
				case GFX::Resource::State::DepthRead:
				case GFX::Resource::State::DepthWrite:
				{
					resDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
					isDS = true;
					break;
				}
				case GFX::Resource::State::UnorderedAccess:
				{
					resDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
					isUA = true;
					break;
				}
				case GFX::Resource::State::ShaderResourcePS:
				case GFX::Resource::State::ShaderResourceNonPS:
				case GFX::Resource::State::ShaderResourceAll:
				{
					isSR = true;
					break;
				}
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

			U64 size = device->GetResourceAllocationInfo(0, 1, &resDesc).SizeInBytes;
			size = size / D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT
				+ static_cast<bool>(size % D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
			resourcesInfo.emplace_back(i, 0, static_cast<U32>(size), lifetime.begin()->first, lifetime.rbegin()->first, resDesc, clearDesc, nullptr,
				static_cast<U8>(isRT) | (isDS << 1) | (isSR << 2) | (isUA << 3) | ((res.Flags & GFX::Pipeline::FrameResourceFlags::Cube) << 4));

			if (isRT)
				++rtvCount;
			else if (isDS)
				++dsvCount;
			if (isSR || isUA)
				++srvUavCount;
		}

		// Prepare data for allocating resources and heaps
		const U64 levelCount = desc.TransitionsPerLevel.size() / 2;
		const U64 invalidID = resourcesInfo.size();
		U64 rt_dsCount;
		U32 maxChunks = 0;
		std::vector<U64> memory;
		D3D12_HEAP_DESC heapDesc;
		heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapDesc.Properties.CreationNodeMask = 0;
		heapDesc.Properties.VisibleNodeMask = 0;
		heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		heapDesc.Flags = static_cast<D3D12_HEAP_FLAGS>(D3D12_HEAP_FLAG_CREATE_NOT_ZEROED
			| D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES);
		std::vector<D3D12_RESOURCE_BARRIER> startingTransitions;
		std::vector<std::pair<U64, D3D12_RESOURCE_BARRIER>> wrappingTransitions;

		// Handle resource types (Non RT/DS) depending on present tier level
		if (dev.Get().dx12.GetCurrentAllocTier() == Device::AllocTier::Tier1)
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
				for (U64 i = rt_dsCount; i < resourcesInfo.size(); ++i)
				{
					auto& res = resourcesInfo.at(i);
					res.Offset = AllocResource(i, res.Chunks, res.StartLevel, res.LastLevel, maxChunksUAV, levelCount, invalidID, memory);
				}

				// Check resource aliasing
				for (U64 i = rt_dsCount; i < resourcesInfo.size(); ++i)
				{
					auto& res = resourcesInfo.at(i);
					if (CheckResourceAliasing(res.Offset, res.Chunks, res.StartLevel,
						res.LastLevel, maxChunksUAV, levelCount, invalidID, memory))
						res.SetAliasing();
				}

				// Find final size for UAV only heap and create it with resources
				heapDesc.SizeInBytes = FindHeapSize(maxChunksUAV, levelCount, invalidID, memory);
				heapDesc.Flags = static_cast<D3D12_HEAP_FLAGS>(D3D12_HEAP_FLAG_CREATE_NOT_ZEROED
					| D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES);
				ZE_GFX_THROW_FAILED(device->CreateHeap(&heapDesc, IID_PPV_ARGS(&uavHeap)));
#ifdef _ZE_DEBUG_FRAME_MEMORY_PRINT
				PrintMemory("T1_UAV", maxChunksUAV, levelCount, invalidID, memory, heapDesc.SizeInBytes);
#endif
				for (U64 i = rt_dsCount; i < resourcesInfo.size(); ++i)
				{
					auto& res = resourcesInfo.at(i);
					const auto& lifetime = desc.ResourceLifetimes.at(res.Handle);
					GFX::Resource::State firstState = lifetime.begin()->second;
					GFX::Resource::State lastState = lifetime.rbegin()->second;
					ZE_GFX_THROW_FAILED(device->CreatePlacedResource(uavHeap.Get(),
						resourcesInfo.at(i).Offset * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
						&res.Desc, GetResourceState(res.IsAliasing() ? firstState : lastState),
						res.IsRTV() || res.IsDSV() ? &res.ClearVal : nullptr, IID_PPV_ARGS(&res.Resource)));
					ZE_GFX_SET_ID(res.Resource, "RID:" + std::to_string(res.Handle));
					if (lastState != firstState)
					{
						U64 lastLevel = 2 * lifetime.rbegin()->first + 1;
						if (res.IsAliasing())
						{
							desc.TransitionsPerLevel.at(lastLevel).emplace_back(res.Handle,
								GFX::Pipeline::BarrierType::Immediate, lastState, firstState);
						}
						else
						{
							desc.TransitionsPerLevel.at(lastLevel).emplace_back(res.Handle,
								GFX::Pipeline::BarrierType::Begin, lastState, firstState);
							D3D12_RESOURCE_BARRIER barrier;
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
					}
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
		for (U64 i = 0; i < rt_dsCount; ++i)
		{
			auto& res = resourcesInfo.at(i);
			res.Offset = AllocResource(i, res.Chunks, res.StartLevel, res.LastLevel, maxChunks, levelCount, invalidID, memory);
		}

		// Check resource aliasing
		for (U64 i = 0; i < rt_dsCount; ++i)
		{
			auto& res = resourcesInfo.at(i);
			if (CheckResourceAliasing(res.Offset, res.Chunks,
				res.StartLevel, res.LastLevel, maxChunks, levelCount, invalidID, memory))
				res.SetAliasing();
		}

		// Find final size for heap and create it with resources
		heapDesc.SizeInBytes = FindHeapSize(maxChunks, levelCount, invalidID, memory);
		heapDesc.Flags = static_cast<D3D12_HEAP_FLAGS>(D3D12_HEAP_FLAG_CREATE_NOT_ZEROED
			| D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES);
		ZE_GFX_THROW_FAILED(device->CreateHeap(&heapDesc, IID_PPV_ARGS(&mainHeap)));
#ifdef _ZE_DEBUG_FRAME_MEMORY_PRINT
		PrintMemory(dev.Get().dx12.GetCurrentAllocTier() == Device::AllocTier::Tier1 ? "T1" : "T2", maxChunks, levelCount, invalidID, memory, heapDesc.SizeInBytes);
#endif
		for (U64 i = 0; i < rt_dsCount; ++i)
		{
			auto& res = resourcesInfo.at(i);
			const auto& lifetime = desc.ResourceLifetimes.at(res.Handle);
			GFX::Resource::State firstState = lifetime.begin()->second;
			GFX::Resource::State lastState = lifetime.rbegin()->second;
			ZE_GFX_THROW_FAILED(device->CreatePlacedResource(mainHeap.Get(),
				resourcesInfo.at(i).Offset * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
				&res.Desc, GetResourceState(res.IsAliasing() ? firstState : lastState),
				res.IsRTV() || res.IsDSV() ? &res.ClearVal : nullptr, IID_PPV_ARGS(&res.Resource)));
			ZE_GFX_SET_ID(res.Resource, "RID:" + std::to_string(res.Handle));
			if (lastState != firstState)
			{
				U64 lastLevel = 2 * lifetime.rbegin()->first + 1;
				if (res.IsAliasing())
				{
					desc.TransitionsPerLevel.at(lastLevel).emplace_back(res.Handle,
						GFX::Pipeline::BarrierType::Immediate, lastState, firstState);
				}
				else
				{
					desc.TransitionsPerLevel.at(lastLevel).emplace_back(res.Handle,
						GFX::Pipeline::BarrierType::Begin, lastState, firstState);
					D3D12_RESOURCE_BARRIER barrier;
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
			}
		}

		// Perform initial transitions for wrapping consistency
		U64 transitionFence = 0;
		if (startingTransitions.size() > 0)
		{
			mainList.Get().dx12.Open(dev);
			mainList.Get().dx12.GetList()->ResourceBarrier(static_cast<U32>(startingTransitions.size()), startingTransitions.data());
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

		// Sort resources for increasing RID
		std::sort(resourcesInfo.begin(), resourcesInfo.end(),
			[](const auto& r1, const auto& r2) -> bool
			{
				return r1.Handle < r2.Handle;
			});
		aliasingResources = new bool[resourcesInfo.size()];
		resources = new DX::ComPtr<ID3D12Resource>[invalidID];
		for (U64 i = 0; auto & res : resourcesInfo)
		{
			aliasingResources[i] = res.IsAliasing();
			resources[i] = std::move(res.Resource);
			++i;
		}

		// Create descriptor heaps
		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc;
		descHeapDesc.NodeMask = 0;
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descHeapDesc.NumDescriptors = rtvCount;
		ZE_GFX_THROW_FAILED(device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&rtvDescHeap)));
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		descHeapDesc.NumDescriptors = dsvCount;
		ZE_GFX_THROW_FAILED(device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&dsvDescHeap)));

		// Get sizes of descriptors
		rtvDsv = new D3D12_CPU_DESCRIPTOR_HANDLE[invalidID + 1];
		srv = new D3D12_CPU_DESCRIPTOR_HANDLE[invalidID + 1];
		uav = new std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE>[invalidID];
		const U32 rtvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		const U32 dsvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		const U32 srvUavDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescHeap->GetCPUDescriptorHandleForHeapStart();
		auto srvUavHandle = dev.Get().dx12.AddStaticDescs(srvUavCount);
		// Create demanded views for each resource
		for (U64 i = 0; const auto & res : resourcesInfo)
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
				ZE_GFX_THROW_FAILED_INFO(device->CreateRenderTargetView(resources[i].Get(), &rtvDesc, rtvHandle));
				rtvDsv[i + 1] = rtvHandle;
				rtvHandle.ptr += rtvDescSize;
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
				ZE_GFX_THROW_FAILED_INFO(device->CreateDepthStencilView(resources[i].Get(), &dsvDesc, dsvHandle));
				rtvDsv[i + 1] = dsvHandle;
				dsvHandle.ptr += dsvDescSize;
			}
			else
				rtvDsv[i + 1].ptr = -1;
			if (res.IsSRV())
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
				srvDesc.Format = DX::ConvertFromDepthStencilFormat(res.Desc.Format);
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				if (res.IsCube())
				{
					if (res.Desc.DepthOrArraySize > 1)
					{
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
						srvDesc.TextureCubeArray.MostDetailedMip = 0;
						srvDesc.TextureCubeArray.MipLevels = res.Desc.MipLevels;
						srvDesc.TextureCubeArray.First2DArrayFace = 0;
						srvDesc.TextureCubeArray.NumCubes = res.Desc.DepthOrArraySize;
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
				ZE_GFX_THROW_FAILED_INFO(device->CreateShaderResourceView(resources[i].Get(), &srvDesc, srvUavHandle.first));
				srv[i + 1] = srvUavHandle.first;
				srvUavHandle.first.ptr += srvUavDescSize;
				srvUavHandle.second.ptr += srvUavDescSize;
			}
			else
				srv[i + 1].ptr = -1;
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
				ZE_GFX_THROW_FAILED_INFO(device->CreateUnorderedAccessView(resources[i].Get(), nullptr, &uavDesc, srvUavHandle.first));
				uav[i] = srvUavHandle;
				srvUavHandle.first.ptr += srvUavDescSize;
				srvUavHandle.second.ptr += srvUavDescSize;
			}
			else
				uav[i].first.ptr = uav[i].second.ptr = -1;
			++i;
		}

		// Find number of barriers and prepare aux structures
		transitions = new TransitionPoint[levelCount];
		U64 barrierCount = 0;
		for (const auto& level : desc.TransitionsPerLevel)
		{
			for (const auto& transition : level)
				if (transition.RID == 0)
					++backbufferBarriersLocationsCount;
			barrierCount += level.size();
		}
		backbufferBarriersLocations = new U64[--backbufferBarriersLocationsCount];
		backbufferBarriersLocations[0] = 0;
		barrierCount += wrappingTransitions.size();
		initTransitions.Barriers = new D3D12_RESOURCE_BARRIER[barrierCount];
		initTransitions.BarrierCount = 0;
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		// Compute barriers before first render passes
		U64 backbufferBarrierIndex = 0;
		auto computeBarriers = [&](TransitionPoint& transitions, const U64 level, const U64 barrierIndex)
		{
			for (const auto& transition : desc.TransitionsPerLevel.at(level))
			{
				barrier.Flags = GetTransitionType(transition.Barrier);
				barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				barrier.Transition.StateBefore = GetResourceState(transition.BeforeState);
				barrier.Transition.StateAfter = GetResourceState(transition.AfterState);
				if (transition.RID == 0)
				{
					if (level != 0)
						backbufferBarriersLocations[backbufferBarrierIndex++] = barrierIndex;
					barrier.Transition.pResource = nullptr;
					transitions.Barriers[transitions.BarrierCount++] = barrier;
				}
				else
				{
					barrier.Transition.pResource = resources[transition.RID - 1].Get();
					transitions.Barriers[transitions.BarrierCount++] = barrier;
				}
			}
			while (wrappingTransitions.size() != 0 && wrappingTransitions.back().first == barrierIndex)
			{
				transitions.Barriers[transitions.BarrierCount++] = wrappingTransitions.back().second;
				wrappingTransitions.pop_back();
			}
		};
		computeBarriers(initTransitions, 0, 0);
		for (auto& wrap : wrappingTransitions)
			--wrap.first;
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
		auto computeBarrierSyncs = [](TransitionPoint& transitions)
		{
			bool copyAfter = false, computeAfter = false;
			for (U32 i = 0; i < transitions.BarrierCount; ++i)
			{
				switch (transitions.Barriers[i].Transition.StateAfter)
				{
				case D3D12_RESOURCE_STATE_GENERIC_READ:
					computeAfter = true;
				case D3D12_RESOURCE_STATE_COMMON:
				{
					copyAfter = true;
					break;
				}
				case D3D12_RESOURCE_STATE_UNORDERED_ACCESS:
				case D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE:
				case D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE:
				{
					computeAfter = true;
					break;
				}
				}
			}
			if (copyAfter && computeAfter)
				transitions.AfterSync = GFX::Pipeline::SyncType::MainToAll;
			else if (copyAfter)
				transitions.AfterSync = GFX::Pipeline::SyncType::MainToCopy;
			else if (computeAfter)
				transitions.AfterSync = GFX::Pipeline::SyncType::MainToCompute;
		};
		computeBarrierSyncs(initTransitions);
		for (U64 i = 0; i < levelCount; ++i)
			computeBarrierSyncs(transitions[i]);

		// Finish initial transitions
		if (startingTransitions.size() > 0)
		{
			dev.Get().dx12.WaitMain(transitionFence);
			mainList.Get().dx12.Reset(dev);
			startingTransitions.clear();
		}
	}

	FrameBuffer::~FrameBuffer()
	{
		if (initTransitions.Barriers)
			delete[] initTransitions.Barriers;
		if (transitions)
			delete[] transitions;
		if (backbufferBarriersLocations)
			delete[] backbufferBarriersLocations;
		if (aliasingResources)
			delete[] aliasingResources;
		if (resources)
			delete[] resources;
		if (rtvDsv)
			delete[] rtvDsv;
		if (srv)
			delete[] srv;
		if (uav)
			delete[] uav;
	}

	void FrameBuffer::SetRTV(GFX::Device& dev, GFX::CommandList& cl, RID rid) const
	{
		ZE_GFX_ENABLE_INFO(dev.Get().dx12);
		ZE_GFX_THROW_FAILED_INFO(cl.Get().dx12.GetList()->OMSetRenderTargets(1, rtvDsv + rid, TRUE, nullptr));
	}

	void FrameBuffer::SetDSV(GFX::Device& dev, GFX::CommandList& cl, RID rid) const
	{
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as depth stencil!");
		ZE_GFX_ENABLE_INFO(dev.Get().dx12);
		ZE_GFX_THROW_FAILED_INFO(cl.Get().dx12.GetList()->OMSetRenderTargets(0, nullptr, TRUE, rtvDsv + rid));
	}

	void FrameBuffer::SetOutput(GFX::Device& dev, GFX::CommandList& cl, RID rtv, RID dsv) const
	{
		ZE_ASSERT(dsv != 0, "Cannot use backbuffer as depth stencil!");
		ZE_GFX_ENABLE_INFO(dev.Get().dx12);
		ZE_GFX_THROW_FAILED_INFO(cl.Get().dx12.GetList()->OMSetRenderTargets(1, rtvDsv + rtv, TRUE, rtvDsv + dsv));
	}

	void FrameBuffer::ClearRTV(GFX::Device& dev, GFX::CommandList& cl, RID rid, const ColorF4 color) const
	{
		ZE_GFX_ENABLE_INFO(dev.Get().dx12);
		ZE_GFX_THROW_FAILED_INFO(cl.Get().dx12.GetList()->ClearRenderTargetView(rtvDsv[rid],
			reinterpret_cast<const float*>(&color), 0, nullptr));
	}

	void FrameBuffer::ClearDSV(GFX::Device& dev, GFX::CommandList& cl, RID rid, float depth, U8 stencil) const
	{
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as depth stencil!");
		ZE_GFX_ENABLE_INFO(dev.Get().dx12);
		ZE_GFX_THROW_FAILED_INFO(cl.Get().dx12.GetList()->ClearDepthStencilView(rtvDsv[rid],
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depth, stencil, 0, nullptr));
	}

	void FrameBuffer::SwapBackbuffer(GFX::Device& dev, GFX::SwapChain& swapChain)
	{
		DX::ComPtr<ID3D12Resource> buffer;
		auto backbufferRtvSrv = swapChain.Get().dx12.SetCurrentBackbuffer(dev, buffer);
		rtvDsv[0] = backbufferRtvSrv.first;
		srv[0] = backbufferRtvSrv.second;
		initTransitions.Barriers->Transition.pResource = buffer.Get();
		for (U64 i = 0; i < backbufferBarriersLocationsCount; ++i)
			transitions[backbufferBarriersLocations[i]].Barriers->Transition.pResource = buffer.Get();
	}

	void FrameBuffer::InitTransitions(GFX::Device& dev, GFX::CommandList& cl) const
	{
		// Perform wrapping barriers
		auto& device = dev.Get().dx12;
		cl.Get().dx12.Open(device);
		cl.Get().dx12.GetList()->ResourceBarrier(initTransitions.BarrierCount, initTransitions.Barriers);
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
		}
	}

	void FrameBuffer::ExitTransitions(GFX::Device& dev, GFX::CommandList& cl, U64 level) const noexcept
	{
		// Perform normal transitions and reseting resources to initial state for aliasing
		auto& transition = transitions[level];
		if (transition.BarrierCount > 0)
		{
			auto& device = dev.Get().dx12;
			cl.Get().dx12.Open(device);
			cl.Get().dx12.GetList()->ResourceBarrier(transition.BarrierCount, transition.Barriers);
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
			}
		}
	}
}