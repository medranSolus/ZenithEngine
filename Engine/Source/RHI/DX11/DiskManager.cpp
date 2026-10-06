#include "RHI/DX11/DiskManager.h"
#include "GFX/GFile.h"
#include "IO/Compressor.h"

namespace ZE::RHI::DX11
{
	void DiskManager::MoveFrom(DiskManager&& disk) noexcept
	{
		currentFenceValue = disk.currentFenceValue;
		baseBucketFenceValue = disk.baseBucketFenceValue;
		statusBuckets = disk.statusBuckets;
	}

	Expected<DiskManager> DiskManager::Create(GFX::Device& dev) noexcept
	{
		DiskManager disk = {};
		disk.statusBuckets.push_back({});
		return disk;
	}

	Expected<DiskStatusHandle> DiskManager::SetGPUUploadWaitPoint() noexcept
	{
		LockGuardRW lock(bucketMutex);
		statusBuckets.push_back({});
		return reinterpret_cast<void*>(currentFenceValue++);
	}

	Status DiskManager::WaitForUploadGPU(GFX::Device& dev, GFX::CommandList& cl, DiskStatusHandle handle) noexcept
	{
		U64 fenceValue = handle.CastPtr<U64>();

		LockGuardRW lock(bucketMutex);
		// Work already checked and finished
		if (fenceValue < baseBucketFenceValue)
			return {};
		ZE_ASSERT(fenceValue < currentFenceValue, "DiskStatusHandle should always contain value smaller than current fence!");

		U64 waitBucketsCount = fenceValue - baseBucketFenceValue + 1;
		do
		{
			for (auto& task : statusBuckets.front())
			{
				Status result = {};
				ZE_EXPECT_RET_FAILED_CODE(result, task.Get());
				ZE_CODE_RET_FAILED(result);
			}
			statusBuckets.pop_front();
		} while (--waitBucketsCount);

		baseBucketFenceValue = fenceValue + 1;
		return {};
	}

	void DiskManager::AddFileBufferRequest(DX::ComPtr<IResource> dest, GFX::GFile& file, U64 sourceOffset,
		U32 sourceBytes, IO::CompressionFormat compression, U32 uncompressedSize) noexcept
	{
		LockGuardRW lock(bucketMutex);
		ZE_ASSERT(statusBuckets.size() > 0, "There should always be at least one bucket!");

		statusBuckets.back().emplace_back(Settings::GetThreadPool().Schedule(ThreadPriority::Normal,
			[](const void* src, U32 srcSize, DX::ComPtr<IResource> dst, U32 dstSize, IO::CompressionFormat compression) noexcept -> Status
			{
				const void* decompressedBuff = nullptr;
				std::unique_ptr<U8[]> decompressedData;
				if (compression != IO::CompressionFormat::None)
				{
					IO::Compressor codec(compression);
					ZE_ASSERT(dstSize == codec.GetOriginalSize(src, srcSize), "Uncompressed sizes don't match!");

					decompressedData = std::make_unique_for_overwrite<U8[]>(dstSize);
					decompressedBuff = decompressedData.get();

					ZE_CODE_RET_FAILED(codec.Decompress(src, srcSize, decompressedData.get(), dstSize));
				}
				else
				{
					ZE_ASSERT(dstSize == srcSize, "Unmatched sizes of buffers for asset!");
					decompressedBuff = src;
				}

				DX::ComPtr<ID3D11Device> dev;
				dst->GetDevice(&dev);
				DX::ComPtr<ID3D11DeviceContext> ctx;
				dev->GetImmediateContext(&ctx);
				ZE_DX_CHECK_FAILED(ctx->UpdateSubresource(dst.Get(), 0, nullptr, decompressedBuff, 0, 0), "There were debug messages during buffer upload!");
				
				return {};
			},
			file.Get().dx11.GetMemory(), sourceBytes, dest, uncompressedSize, compression));
	}

	void DiskManager::AddFileTextureRequest(DX::ComPtr<IResource> dest, GFX::GFile& file, U64 sourceOffset,
		U32 sourceBytes, IO::CompressionFormat compression, U32 uncompressedSize, U32 rowPitch, U32 depthPitch) noexcept
	{
		LockGuardRW lock(bucketMutex);
		ZE_ASSERT(statusBuckets.size() > 0, "There should always be at least one bucket!");

		statusBuckets.back().emplace_back(Settings::GetThreadPool().Schedule(ThreadPriority::Normal,
			[](const void* src, U32 srcSize, DX::ComPtr<IResource> dst, U32 dstSize, IO::CompressionFormat compression, U32 rowPitch, U32 depthPitch) noexcept -> Status
			{
				const void* decompressedBuff = nullptr;
				std::unique_ptr<U8[]> decompressedData;
				if (compression != IO::CompressionFormat::None)
				{
					IO::Compressor codec(compression);
					ZE_ASSERT(dstSize == codec.GetOriginalSize(src, srcSize), "Uncompressed sizes don't match!");

					decompressedData = std::make_unique_for_overwrite<U8[]>(dstSize);
					decompressedBuff = decompressedData.get();

					ZE_CODE_RET_FAILED(codec.Decompress(src, srcSize, decompressedData.get(), dstSize));
				}
				else
				{
					ZE_ASSERT(dstSize == srcSize, "Unmatched sizes of buffers for asset!");
					decompressedBuff = src;
				}

				DX::ComPtr<ID3D11Device> dev;
				dst->GetDevice(&dev);
				DX::ComPtr<ID3D11DeviceContext> ctx;
				dev->GetImmediateContext(&ctx);
				// TODO: Maybe need to do it per subresource too?...
				ZE_DX_CHECK_FAILED(ctx->UpdateSubresource(dst.Get(), 0, nullptr, decompressedBuff, rowPitch, depthPitch), "There were debug messages during buffer upload!");

				return {};
			},
			file.Get().dx11.GetMemory(), sourceBytes, dest, uncompressedSize, compression, rowPitch, depthPitch));
	}
}