#include "RHI/DX12/DiskManager.h"
#include "Data/ResourceLocation.h"
#include "IO/Compressor.h"
#include "IO/File.h"
ZE_WARNING_PUSH
#include <winioctl.h>
ZE_WARNING_POP

namespace ZE::RHI::DX12
{
	constexpr DSTORAGE_COMPRESSION_FORMAT DiskManager::GetCompressionFormat(IO::CompressionFormat compression) noexcept
	{
		switch (compression)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case IO::CompressionFormat::None:
			return DSTORAGE_CUSTOM_COMPRESSION_0;
		case IO::CompressionFormat::ZLib:
			return COMPRESSION_FORMAT_ZLIB;
		}
	}

	bool DiskManager::IsFileOnSSD(std::wstring_view path) noexcept
	{
		wchar_t volumePath[MAX_PATH];
		if (!GetVolumePathNameW(path.data(), volumePath, ARRAYSIZE(volumePath)))
			return false;

		wchar_t volumeName[MAX_PATH];
		if (!GetVolumeNameForVolumeMountPointW(volumePath, volumeName, ARRAYSIZE(volumeName)))
			return false;

		size_t length = wcslen(volumeName);
		if (length && volumeName[length - 1] == L'\\')
			volumeName[length - 1] = L'\0';

		HANDLE volume = CreateFileW(volumeName, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
		// Check for invalid path
		if (volume == INVALID_HANDLE_VALUE)
			return false;

		// Perform query for SSD (no seek penalty)
		STORAGE_PROPERTY_QUERY query = {};
		query.PropertyId = StorageDeviceSeekPenaltyProperty;
		query.QueryType = PropertyStandardQuery;

		bool isSSD;
		DWORD count;
		DEVICE_SEEK_PENALTY_DESCRIPTOR result;
		if (DeviceIoControl(volume, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(STORAGE_PROPERTY_QUERY), &result, sizeof(DEVICE_SEEK_PENALTY_DESCRIPTOR), &count, nullptr))
			isSSD = !result.IncursSeekPenalty;
		else
			isSSD = false;

		CloseHandle(volume);
		return isSSD;
	}

	void DiskManager::DecompressAssets(Device& dev) const
	{
		ZE_DX_ENABLE(dev);

		ThreadPool& pool = Settings::GetThreadPool();
		const U8 maxRequestCount = std::max(pool.GetWorkerThreadsCount(), static_cast<U8>(MINIMAL_DECOPRESSED_OBJECTS_PER_TURN));
		auto requests = std::make_unique<DSTORAGE_CUSTOM_DECOMPRESSION_REQUEST[]>(maxRequestCount);
		auto results = std::make_unique<DSTORAGE_CUSTOM_DECOMPRESSION_RESULT[]>(maxRequestCount);
		auto decompresionTasks = std::make_unique<Task<void>[]>(maxRequestCount);

		while (checkForDecompression)
		{
			// Check if any requests ready with timeout for checking of program end
			switch (WaitForSingleObject(decompressionEvent, MAX_DECOMPRESSION_WAIT))
			{
			case WAIT_TIMEOUT:
				continue;
			case WAIT_OBJECT_0:
				break;
			case WAIT_ABANDONED:
				ZE_FAIL("Error occured in thread releasing DirectStorage decompression queue mutex!");
			default:
				throw ZE_WIN_EXCEPT_LAST();
			}

			// Process custom formats first (with well known implementation of decompression)
			U32 requestCount = 0;
			ZE_DX_THROW_FAILED(decompressQueue->GetRequests1(DSTORAGE_GET_REQUEST_FLAG_SELECT_CUSTOM, maxRequestCount, requests.get(), &requestCount));
			for (U32 i = 0; i < requestCount; ++i)
			{
				DSTORAGE_CUSTOM_DECOMPRESSION_REQUEST& req = requests[i];
				DSTORAGE_CUSTOM_DECOMPRESSION_RESULT& res = results[i];
				switch (req.CompressionFormat)
				{
				case DSTORAGE_CUSTOM_COMPRESSION_0: // Custom format is currently with no compression
				{
					ZE_ASSERT(req.DstSize == req.SrcSize, "Unmatched sizes of buffers for asset!");
					decompresionTasks[i] = pool.Schedule(ThreadPriority::Normal,
						[](void* dst, const void* src, U64 size) { std::memcpy(dst, src, size); },
						req.DstBuffer, req.SrcBuffer, req.DstSize);
					break;
				}
				ZE_WARNING_PUSH;
				ZE_WARNING_DISABLE_MSVC(4063);
				case COMPRESSION_FORMAT_ZLIB:
				{
					decompresionTasks[i] = pool.Schedule(ThreadPriority::Normal,
						[](const void* src, U32 srcSize, void* dst, U32 dstSize)
						{
							IO::Compressor codec(IO::CompressionFormat::ZLib);
							ZE_ASSERT(dstSize == codec.GetOriginalSize(src, srcSize), "Uncompressed sizes don't match!");
							codec.Decompress(src, srcSize, dst, dstSize);
						},
						req.SrcBuffer, Utils::SafeCast<U32>(req.SrcSize), req.DstBuffer, Utils::SafeCast<U32>(req.DstSize));
					break;
				}
				ZE_WARNING_POP;
				default:
					ZE_FAIL("Unknown type of custom decompression request!");
					break;
				}
				res.Id = req.Id;
				res.Result = S_OK;
			}

			// Process built-in formats with codecs
			U32 builtInRequestCount = 0;
			ZE_DX_THROW_FAILED(decompressQueue->GetRequests1(DSTORAGE_GET_REQUEST_FLAG_SELECT_BUILTIN,
				maxRequestCount - requestCount, requests.get() + requestCount, &builtInRequestCount));
			for (U32 i = 0; i < builtInRequestCount; ++i)
			{
				DSTORAGE_CUSTOM_DECOMPRESSION_REQUEST& req = requests[i + requestCount];
				DSTORAGE_CUSTOM_DECOMPRESSION_RESULT& res = results[i + requestCount];
				switch (req.CompressionFormat)
				{
				case DSTORAGE_COMPRESSION_FORMAT_GDEFLATE:
				{
					size_t dataWritten = 0;
					ZE_DX_THROW_FAILED(compressCodecGDeflate->DecompressBuffer(req.SrcBuffer, req.SrcSize, req.DstBuffer, req.DstSize, &dataWritten));
					break;
				}
				default:
					ZE_FAIL("Unknown type of built-in decompression request!");
					break;
				}
				res.Id = req.Id;
				res.Result = S_OK;
			}

			// Wait for custom requests to complete
			for (U32 i = 0; i < requestCount; ++i)
				decompresionTasks[i].Get();

			ZE_DX_THROW_FAILED(decompressQueue->SetRequestResults(requestCount + builtInRequestCount, results.get()));
		}
	}

	void DiskManager::AddRequest(EID resourceID, IResource* dest, ResourceType type, std::shared_ptr<const U8[]> src) noexcept
	{
		LockGuardRW lock(queueMutex, resourceID != INVALID_EID || dest || src);
		if (dest)
			uploadDestResourceQueue.emplace_back(type, dest);
		if (src)
			uploadSrcMemoryQueue.emplace_back(src);
		if (resourceID != INVALID_EID)
		{
#if _ZE_MODE_DEBUG || _ZE_MODE_DEV
			for (EID id : uploadQueue)
			{
				ZE_ASSERT(id != resourceID, "Same resource EID added twice as request for DirectStorage upload!");
			}
#endif
			uploadQueue.emplace_back(resourceID);
			Settings::Data.get_or_emplace<Data::ResourceLocationAtom>(resourceID) = Data::ResourceLocation::UploadingToGPU;
		}
	}

	DiskManager::DiskManager(GFX::Device& dev)
	{
		ZE_DX_ENABLE_ID(dev.Get().dx12);
		IDevice* device = dev.Get().dx12.GetDevice();

		DWORD pathLen = GetCurrentDirectoryW(0, nullptr);
		std::wstring currentPath(pathLen, L'\0');
		bool isHDD;
		if (GetCurrentDirectoryW(pathLen, currentPath.data()) == 0)
		{
			ZE_WARNING("Cannot determine if current disk is SSD or HDD, falling back to HDD path for DirectStorage");
			isHDD = true;
		}
		else
			isHDD = !IsFileOnSSD(currentPath);

		DSTORAGE_CONFIGURATION1 dsConfig = {};
		dsConfig.NumSubmitThreads = 0;
		dsConfig.NumBuiltInCpuDecompressionThreads = DSTORAGE_DISABLE_BUILTIN_CPU_DECOMPRESSION;
		dsConfig.ForceMappingLayer = false;
		dsConfig.DisableBypassIO = isHDD;
		dsConfig.DisableTelemetry = true;
		dsConfig.DisableGpuDecompressionMetacommand = false;
		dsConfig.DisableGpuDecompression = false;
		dsConfig.ForceFileBuffering = isHDD;
		ZE_DX_THROW_FAILED(DStorageSetConfiguration1(&dsConfig));
		ZE_DX_THROW_FAILED(DStorageGetFactory(IID_PPV_ARGS(&factory)));
#if _ZE_DEBUG_GFX_API
		factory->SetDebugFlags(DSTORAGE_DEBUG_SHOW_ERRORS | DSTORAGE_DEBUG_BREAK_ON_ERROR | (_ZE_DEBUG_GFX_NAMES ? DSTORAGE_DEBUG_RECORD_OBJECT_NAMES : 0));
#endif
		factory->SetStagingBufferSize(Settings::STAGING_BUFFER_SIZE);

		ZE_DX_THROW_FAILED(factory.As(&decompressQueue));
		decompressionEvent = decompressQueue->GetEvent();

		DSTORAGE_QUEUE_DESC queueDesc = {};
		queueDesc.SourceType = DSTORAGE_REQUEST_SOURCE_FILE;
		queueDesc.Capacity = DSTORAGE_MAX_QUEUE_CAPACITY;
		queueDesc.Priority = DSTORAGE_PRIORITY_NORMAL;
#if _ZE_DEBUG_GFX_NAMES
		queueDesc.Name = "DirectStorage file queue";
#endif
		queueDesc.Device = device;
		ZE_DX_THROW_FAILED(factory->CreateQueue(&queueDesc, IID_PPV_ARGS(&fileQueue)));

		queueDesc.SourceType = DSTORAGE_REQUEST_SOURCE_MEMORY;
#if _ZE_DEBUG_GFX_NAMES
		queueDesc.Name = "DirectStorage memory queue";
#endif
		ZE_DX_THROW_FAILED(factory->CreateQueue(&queueDesc, IID_PPV_ARGS(&memoryQueue)));
		ZE_DX_THROW_FAILED(DStorageCreateCompressionCodec(DSTORAGE_COMPRESSION_FORMAT_GDEFLATE, 0, IID_PPV_ARGS(&compressCodecGDeflate)));

		fenceEvents[0] = CreateEventW(nullptr, false, false, nullptr);
		ZE_ASSERT(fenceEvents[0], "Cannot create DirectStorage file queue fence event!");
		fenceEvents[1] = CreateEventW(nullptr, false, false, nullptr);
		ZE_ASSERT(fenceEvents[1], "Cannot create DirectStorage memory queue fence event!");
		cpuDecompressionThread = std::jthread([this, &dev]() { this->DecompressAssets(dev.Get().dx12); });
	}

	DiskManager::~DiskManager()
	{
		checkForDecompression = false;
		if (fenceEvents[0])
			CloseHandle(fenceEvents[0]);
		if (fenceEvents[1])
			CloseHandle(fenceEvents[1]);
		if (decompressionEvent)
			CloseHandle(decompressionEvent);
	}

	void DiskManager::StartUploadGPU(bool waitable) noexcept
	{
		if (waitable)
		{
			fileQueue->EnqueueSetEvent(fenceEvents[0]);
			memoryQueue->EnqueueSetEvent(fenceEvents[1]);
		}
		fileQueue->Submit();
		memoryQueue->Submit();

		LockGuardRW lock(queueMutex);
		submitQueue.reserve(submitQueue.size() + uploadQueue.size());
		submitQueue.insert(submitQueue.end(), uploadQueue.begin(), uploadQueue.end());
		uploadQueue.clear();

		submitDestResourceQueue.reserve(submitDestResourceQueue.size() + uploadDestResourceQueue.size());
		submitDestResourceQueue.insert(submitDestResourceQueue.end(), uploadDestResourceQueue.begin(), uploadDestResourceQueue.end());
		uploadDestResourceQueue.clear();

		submitSrcMemoryQueue.reserve(submitSrcMemoryQueue.size() + uploadSrcMemoryQueue.size());
		submitSrcMemoryQueue.insert(submitSrcMemoryQueue.end(), uploadSrcMemoryQueue.begin(), uploadSrcMemoryQueue.end());
		uploadSrcMemoryQueue.clear();
	}

	bool DiskManager::WaitForUploadGPU(GFX::Device& dev, GFX::CommandList& cl)
	{
		if (WaitForMultipleObjects(2, fenceEvents, true, INFINITE) != WAIT_OBJECT_0)
			throw ZE_WIN_EXCEPT_LAST();

		DSTORAGE_ERROR_RECORD errorRecord = {};
		fileQueue->RetrieveErrorRecord(&errorRecord);

		if (SUCCEEDED(errorRecord.FirstFailure.HResult))
		{
			ZE_DX_ENABLE_INFO(dev.Get().dx12);

			LockGuardRW lock(queueMutex);
			submitSrcMemoryQueue.clear();

			// Set all resources location to GPU
			for (EID id : submitQueue)
				Settings::Data.get<Data::ResourceLocationAtom>(id) = Data::ResourceLocation::GPU;
			submitQueue.clear();

			// Transition all textures to SRV and perform buffer barriers
			std::vector<D3D12_TEXTURE_BARRIER> textureBarriers;
			std::vector<D3D12_BUFFER_BARRIER> bufferBarriers;
			for (auto& res : submitDestResourceQueue)
			{
				if (res.first == ResourceType::Texture)
				{
					D3D12_TEXTURE_BARRIER& barrier = textureBarriers.emplace_back();
					barrier.SyncBefore = D3D12_BARRIER_SYNC_ALL;
					barrier.SyncAfter = D3D12_BARRIER_SYNC_ALL_SHADING;
					barrier.AccessBefore = D3D12_BARRIER_ACCESS_COMMON;
					barrier.AccessAfter = D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
					barrier.LayoutBefore = D3D12_BARRIER_LAYOUT_COMMON;
					barrier.LayoutAfter = D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
					barrier.pResource = res.second;
					barrier.Subresources.IndexOrFirstMipLevel = UINT32_MAX;
					barrier.Subresources.NumMipLevels = 0;
					barrier.Subresources.FirstArraySlice = 0;
					barrier.Subresources.NumArraySlices = 0;
					barrier.Subresources.FirstPlane = 0;
					barrier.Subresources.NumPlanes = 0;
					barrier.Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE;
				}
				else
				{
					D3D12_BUFFER_BARRIER& barrier = bufferBarriers.emplace_back();
					barrier.SyncBefore = D3D12_BARRIER_SYNC_ALL;
					barrier.SyncAfter = D3D12_BARRIER_SYNC_ALL_SHADING;
					barrier.AccessBefore = D3D12_BARRIER_ACCESS_COMMON;
					barrier.AccessAfter = res.first == ResourceType::Buffer ? D3D12_BARRIER_ACCESS_CONSTANT_BUFFER : D3D12_BARRIER_ACCESS_INDEX_BUFFER | D3D12_BARRIER_ACCESS_VERTEX_BUFFER;
					barrier.pResource = res.second;
					barrier.Offset = 0;
					barrier.Size = UINT64_MAX;
				}
			}
			submitDestResourceQueue.clear();

			U8 index = 0;
			D3D12_BARRIER_GROUP barrierGroups[2];
			if (textureBarriers.size())
			{
				barrierGroups[0].Type = D3D12_BARRIER_TYPE_TEXTURE;
				barrierGroups[0].NumBarriers = Utils::SafeCast<U32>(textureBarriers.size());
				barrierGroups[0].pTextureBarriers = textureBarriers.data();
				++index;
			}
			if (bufferBarriers.size())
			{
				barrierGroups[index].Type = D3D12_BARRIER_TYPE_BUFFER;
				barrierGroups[index].NumBarriers = Utils::SafeCast<U32>(bufferBarriers.size());
				barrierGroups[index].pBufferBarriers = bufferBarriers.data();
				++index;
			}
			if (index)
			{
				ZE_DX_THROW_FAILED_INFO(cl.Get().dx12.GetList()->Barrier(index, barrierGroups));
			}
			return true;
		}
		ZE_WARNING("The DirectStorage request failed!");
		return false;
	}

	void DiskManager::AddFileBufferRequest(EID resourceID, IResource* dest, IO::File& file, U64 sourceOffset,
		U32 sourceBytes, IO::CompressionFormat compression, U32 uncompressedSize, bool isMesh) noexcept
	{
		ZE_ASSERT(dest, "Empty destination resource!");
		ZE_ASSERT(sourceBytes, "Zero sized source buffer!");
		ZE_ASSERT(uncompressedSize, "Zero sized destination buffer!");
		ZE_ASSERT(sourceBytes <= Settings::STAGING_BUFFER_SIZE, "Size of file buffer exceedes size of staging buffer! Max size: "
			+ std::to_string(Settings::STAGING_BUFFER_SIZE) + " MB, provided: " + std::to_string(sourceBytes / Math::MEGABYTE));

		DSTORAGE_REQUEST request = {};
		request.Options.CompressionFormat = GetCompressionFormat(compression);
		request.Options.SourceType = DSTORAGE_REQUEST_SOURCE_FILE;
		request.Options.DestinationType = DSTORAGE_REQUEST_DESTINATION_BUFFER;

		request.Source.File.Source = file.Get().dx12.GetStorageFile();
		request.Source.File.Offset = sourceOffset;
		request.Source.File.Size = sourceBytes;

		request.Destination.Buffer.Resource = dest;
		request.Destination.Buffer.Offset = 0;
		request.Destination.Buffer.Size = uncompressedSize;

		request.UncompressedSize = uncompressedSize;
		request.CancellationTag = 0;

		fileQueue->EnqueueRequest(&request);
		AddRequest(resourceID, dest, isMesh ? ResourceType::Mesh : ResourceType::Buffer, nullptr);
	}

	void DiskManager::AddMemoryBufferRequest(EID resourceID, IResource* dest, const void* srcStatic,
		std::shared_ptr<const U8[]> srcCopy, U32 bytes, bool isMesh) noexcept
	{
		ZE_ASSERT(dest, "Empty destination resource!");
		ZE_ASSERT(srcStatic || srcCopy, "Empty source buffer!");
		ZE_ASSERT((srcStatic == nullptr) != (srcCopy == nullptr), "Only single source type have to be provided!");
		ZE_ASSERT(bytes, "Zero sized source buffer!");
		ZE_ASSERT(bytes <= Settings::STAGING_BUFFER_SIZE, "Size of buffer exceedes size of staging buffer! Max size: "
			+ std::to_string(Settings::STAGING_BUFFER_SIZE) + " MB, provided: " + std::to_string(bytes / Math::MEGABYTE));

		DSTORAGE_REQUEST request = {};
		request.Options.CompressionFormat = DSTORAGE_CUSTOM_COMPRESSION_0;
		request.Options.SourceType = DSTORAGE_REQUEST_SOURCE_MEMORY;
		request.Options.DestinationType = DSTORAGE_REQUEST_DESTINATION_BUFFER;

		request.Source.Memory.Source = srcCopy.get() ? srcCopy.get() : srcStatic;
		request.Source.Memory.Size = bytes;

		request.Destination.Buffer.Resource = dest;
		request.Destination.Buffer.Offset = 0;
		request.Destination.Buffer.Size = bytes;

		request.UncompressedSize = bytes;
		request.CancellationTag = 0;

		memoryQueue->EnqueueRequest(&request);
		AddRequest(resourceID, dest, isMesh ? ResourceType::Mesh : ResourceType::Buffer, srcCopy);
	}

	void DiskManager::AddMemoryTextureRequest(IResource* dest, std::shared_ptr<const U8[]> src, U32 bytes) noexcept
	{
		ZE_ASSERT(dest, "Empty destination resource!");
		ZE_ASSERT(src, "Empty source texture!");
		ZE_ASSERT(bytes, "Zero sized source texture buffer!");
		ZE_ASSERT(bytes <= Settings::STAGING_BUFFER_SIZE, "Size of texture exceedes size of staging buffer! Max size: "
			+ std::to_string(Settings::STAGING_BUFFER_SIZE) + " MB, provided: " + std::to_string(bytes / Math::MEGABYTE));

		DSTORAGE_REQUEST request = {};
		request.Options.CompressionFormat = DSTORAGE_CUSTOM_COMPRESSION_0;
		request.Options.SourceType = DSTORAGE_REQUEST_SOURCE_MEMORY;
		request.Options.DestinationType = DSTORAGE_REQUEST_DESTINATION_MULTIPLE_SUBRESOURCES;

		request.Source.Memory.Source = src.get();
		request.Source.Memory.Size = bytes;

		request.Destination.MultipleSubresources.Resource = dest;
		request.Destination.MultipleSubresources.FirstSubresource = 0;

		request.UncompressedSize = bytes;
		request.CancellationTag = 0;

		memoryQueue->EnqueueRequest(&request);
		AddRequest(INVALID_EID, dest, ResourceType::Texture, src);
	}

	void DiskManager::AddMemoryTextureArrayRequest(IResource* dest, std::shared_ptr<const U8[]> src,
		U32 bytes, U16 arrayIndex, U32 width, U32 height, bool lastElement) noexcept
	{
		ZE_ASSERT(dest, "Empty destination resource!");
		ZE_ASSERT(src, "Empty source texture!");
		ZE_ASSERT(bytes, "Zero sized source texture buffer!");
		ZE_ASSERT(width && height, "Empty texture dimensions!");
		ZE_ASSERT(bytes <= Settings::STAGING_BUFFER_SIZE, "Size of texture in array exceedes size of staging buffer! Max size: "
			+ std::to_string(Settings::STAGING_BUFFER_SIZE) + " MB, provided: " + std::to_string(bytes / Math::MEGABYTE));

		DSTORAGE_REQUEST request = {};
		request.Options.CompressionFormat = DSTORAGE_CUSTOM_COMPRESSION_0;
		request.Options.SourceType = DSTORAGE_REQUEST_SOURCE_MEMORY;
		request.Options.DestinationType = DSTORAGE_REQUEST_DESTINATION_TEXTURE_REGION;

		request.Source.Memory.Source = src.get();
		request.Source.Memory.Size = bytes;

		request.Destination.Texture.Resource = dest;
		request.Destination.Texture.SubresourceIndex = arrayIndex;
		request.Destination.Texture.Region.left = 0;
		request.Destination.Texture.Region.top = 0;
		request.Destination.Texture.Region.front = 0;
		request.Destination.Texture.Region.right = width;
		request.Destination.Texture.Region.bottom = height;
		request.Destination.Texture.Region.back = 1;

		request.UncompressedSize = bytes;
		request.CancellationTag = 0;

		memoryQueue->EnqueueRequest(&request);
		AddRequest(INVALID_EID, lastElement ? dest : nullptr, ResourceType::Texture, src);
	}
}