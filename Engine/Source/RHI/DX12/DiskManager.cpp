#include "RHI/DX12/DiskManager.h"
ZE_WARNING_PUSH
#include <winioctl.h>
ZE_WARNING_POP

namespace ZE::RHI::DX12
{
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

		cpuDecompressionThread = std::thread([this, &dev]() { this->DecompressAssets(dev.Get().dx12); });
	}

	DiskManager::~DiskManager()
	{
		checkForDecompression = false;
		cpuDecompressionThread.join();
	}
}