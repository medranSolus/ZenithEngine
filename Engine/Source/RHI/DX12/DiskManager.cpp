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
		const U8 maxRequestCount = pool.GetThreadsCount();
		auto requests = std::make_unique<DSTORAGE_CUSTOM_DECOMPRESSION_REQUEST[]>(maxRequestCount);
		auto results = std::make_unique<DSTORAGE_CUSTOM_DECOMPRESSION_RESULT[]>(maxRequestCount);
		auto decompresionTasks = std::make_unique<Task<void>[]>(maxRequestCount);

		while (checkForDecompression)
		{
			// Check if any requests ready with timeout for checking of program end
			switch (WaitForSingleObject(decompressQueue->GetEvent(), MAX_DECOMPRESSION_WAIT))
			{
			case WAIT_TIMEOUT:
				continue;
			case WAIT_OBJECT_0:
				break;
			case WAIT_ABANDONED:
				ZE_FAIL("Error occured by thread releasing DirectStorage decompression queue mutex!");
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
						[](void* dst, const void* src, U64 size) { memcpy(dst, src, size); },
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
			ZE_DX_THROW_FAILED(decompressQueue->GetRequests1(DSTORAGE_GET_REQUEST_FLAG_SELECT_BUILTIN, maxRequestCount - requestCount, requests.get() + requestCount, &builtInRequestCount));
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
		ZE_DX_THROW_FAILED(factory.As(&decompressQueue));

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

		cpuDecompressionThread = std::jthread([this, &dev]() { this->DecompressAssets(dev.Get().dx12); });
	}

	DiskManager::~DiskManager()
	{
		checkForDecompression = false;
		if (fenceEvents[0])
			CloseHandle(fenceEvents[0]);
		if (fenceEvents[1])
			CloseHandle(fenceEvents[1]);
	}

	void DiskManager::StartUploadGPU(bool waitable) noexcept
	{
		if (waitable)
		{
			fenceEvents[0] = CreateEventW(nullptr, false, false, nullptr);
			ZE_ASSERT(fenceEvents[0], "Cannot create DirectStorage file queue fence event!");
			fileQueue->EnqueueSetEvent(fenceEvents[0]);

			fenceEvents[1] = CreateEventW(nullptr, false, false, nullptr);
			ZE_ASSERT(fenceEvents[1], "Cannot create DirectStorage file queue fence event!");
			memoryQueue->EnqueueSetEvent(fenceEvents[1]);
		}
		fileQueue->Submit();
		memoryQueue->Submit();

		LockGuardRW lock(queueMutex);
		submitQueue.reserve(submitQueue.size() + uploadQueue.size());
		submitQueue.insert(submitQueue.end(), uploadQueue.begin(), uploadQueue.end());
		uploadQueue.clear();

		submitSrcMemoryQueue.reserve(submitSrcMemoryQueue.size() + uploadSrcMemoryQueue.size());
		submitSrcMemoryQueue.insert(submitSrcMemoryQueue.end(), uploadSrcMemoryQueue.begin(), uploadSrcMemoryQueue.end());
		uploadSrcMemoryQueue.clear();
	}

	bool DiskManager::WaitForUploadGPU()
	{
		ZE_ASSERT(fenceEvents[0] != nullptr && fenceEvents[1] != nullptr,
			"Cannot wait for upload to complete when it hasn't been started in waitable mode!");

		if (WaitForMultipleObjects(2, fenceEvents, true, INFINITE) != WAIT_OBJECT_0)
			throw ZE_WIN_EXCEPT_LAST();
		if (CloseHandle(fenceEvents[0]) == 0)
			throw ZE_WIN_EXCEPT_LAST();
		if (CloseHandle(fenceEvents[1]) == 0)
			throw ZE_WIN_EXCEPT_LAST();
		fenceEvents[0] = nullptr;
		fenceEvents[1] = nullptr;
		submitSrcMemoryQueue.clear();

		DSTORAGE_ERROR_RECORD errorRecord = {};
		fileQueue->RetrieveErrorRecord(&errorRecord);

		if (SUCCEEDED(errorRecord.FirstFailure.HResult))
		{
			// Set all resources location to GPU
			for (EID id : submitQueue)
				Settings::Data.get<Data::ResourceLocationAtom>(id) = Data::ResourceLocation::GPU;
			return true;
		}
		ZE_WARNING("The DirectStorage request failed!");
		return false;
	}

	void DiskManager::AddFileBufferRequest(EID resourceID, IO::File& file, IResource* dest, U64 sourceOffset,
		U32 sourceBytes, IO::CompressionFormat compression, U32 uncompressedSize) noexcept
	{
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
		if (resourceID != INVALID_EID)
		{
			Settings::Data.get<Data::ResourceLocationAtom>(resourceID) = Data::ResourceLocation::UploadingToGPU;

			LockGuardRW lock(queueMutex);
			uploadQueue.emplace_back(resourceID);
		}
	}

	void DiskManager::AddMemoryBufferRequest(EID resourceID, IResource* dest, const void* src, U32 bytes) noexcept
	{
		DSTORAGE_REQUEST request = {};
		request.Options.CompressionFormat = DSTORAGE_COMPRESSION_FORMAT_NONE;
		request.Options.SourceType = DSTORAGE_REQUEST_SOURCE_MEMORY;
		request.Options.DestinationType = DSTORAGE_REQUEST_DESTINATION_BUFFER;

		request.Source.Memory.Source = src;
		request.Source.Memory.Size = bytes;

		request.Destination.Buffer.Resource = dest;
		request.Destination.Buffer.Offset = 0;
		request.Destination.Buffer.Size = bytes;

		request.UncompressedSize = bytes;
		request.CancellationTag = 0;

		memoryQueue->EnqueueRequest(&request);
		if (resourceID != INVALID_EID)
		{
			Settings::Data.get<Data::ResourceLocationAtom>(resourceID) = Data::ResourceLocation::UploadingToGPU;

			LockGuardRW lock(queueMutex);
			uploadQueue.emplace_back(resourceID);
		}
	}

	void DiskManager::AddMemorySingleTextureRequest(EID resourceID, IResource* dest, std::shared_ptr<U8[]> src, U32 bytes) noexcept
	{
		DSTORAGE_REQUEST request = {};
		request.Options.CompressionFormat = DSTORAGE_COMPRESSION_FORMAT_NONE;
		request.Options.SourceType = DSTORAGE_REQUEST_SOURCE_MEMORY;
		request.Options.DestinationType = DSTORAGE_REQUEST_DESTINATION_TEXTURE_REGION;

		request.Source.Memory.Source = src.get();
		request.Source.Memory.Size = bytes;

		request.Destination.Texture.Resource = dest;
		request.Destination.Texture.SubresourceIndex = 0;
		request.Destination.Texture.Region;

		request.UncompressedSize = bytes;
		request.CancellationTag = 0;

		memoryQueue->EnqueueRequest(&request);

		LockGuardRW lock(queueMutex);
		uploadSrcMemoryQueue.emplace_back(std::move(src));
		if (resourceID != INVALID_EID)
		{
			uploadQueue.emplace_back(resourceID);
			Settings::Data.get<Data::ResourceLocationAtom>(resourceID) = Data::ResourceLocation::UploadingToGPU;
		}
	}
}