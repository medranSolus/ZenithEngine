#include "RHI/DX11/DiskManager.h"
#include "Data/ResourceLocation.h"
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

	void DiskManager::AddFileBufferRequest(EID resourceID, DX::ComPtr<IResource> dest, GFX::GFile& file, U64 sourceOffset,
		U32 sourceBytes, IO::CompressionFormat compression, U32 uncompressedSize) noexcept
	{
		if (resourceID != INVALID_EID)
			Settings::Data.get_or_emplace<Data::ResourceLocationAtom>(resourceID) = Data::ResourceLocation::UploadingToGPU;

		LockGuardRW lock(bucketMutex);
		ZE_ASSERT(statusBuckets.size() > 0, "There should always be at least one bucket!");

		statusBuckets.back().emplace_back(Settings::GetThreadPool().Schedule(ThreadPriority::Normal,
			[](const void* src, U32 srcSize, DX::ComPtr<IResource> dst, U32 dstSize, IO::CompressionFormat compression, EID resourceID) noexcept -> Status
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
				
				if (resourceID != INVALID_EID)
					Settings::Data.get_or_emplace<Data::ResourceLocationAtom>(resourceID) = Data::ResourceLocation::GPU;
				return {};
			},
			file.Get().dx11.GetMemory(), sourceBytes, dest, uncompressedSize, compression, resourceID));
	}

	void DiskManager::AddFileTextureRequest(std::latch* barrier, DX::ComPtr<IResource> dest, GFX::GFile& file, U64 sourceOffset,
		U32 sourceBytes, IO::CompressionFormat compression, U32 uncompressedSize) noexcept
	{
		LockGuardRW lock(bucketMutex);
		ZE_ASSERT(statusBuckets.size() > 0, "There should always be at least one bucket!");

		statusBuckets.back().emplace_back(Settings::GetThreadPool().Schedule(ThreadPriority::Normal,
			[](const void* src, U32 srcSize, DX::ComPtr<IResource> dst, U32 dstSize, IO::CompressionFormat compression, std::latch* barrier) noexcept -> Status
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

				if (barrier)
					barrier->count_down();
				return {};
			},
			file.Get().dx11.GetMemory(), sourceBytes, dest, uncompressedSize, compression, barrier));
	}

	void DiskManager::AddTexturePackID(EID resourceID, std::unique_ptr<std::latch> barrier) noexcept
	{
		if (resourceID != INVALID_EID)
		{
			if (barrier->try_wait())
				Settings::Data.get_or_emplace<Data::ResourceLocationAtom>(resourceID) = Data::ResourceLocation::GPU;
			else
			{
				Settings::Data.get_or_emplace<Data::ResourceLocationAtom>(resourceID) = Data::ResourceLocation::UploadingToGPU;

				LockGuardRW lock(bucketMutex);
				ZE_ASSERT(statusBuckets.size() > 0, "There should always be at least one bucket!");

				statusBuckets.back().emplace_back(Settings::GetThreadPool().Schedule(ThreadPriority::Normal,
					[barrier = std::move(barrier)](EID resourceID) noexcept -> Status
					{
						barrier->wait();
						Settings::Data.get_or_emplace<Data::ResourceLocationAtom>(resourceID) = Data::ResourceLocation::GPU;
						return {};
					},
					resourceID));
			}
		}
	}
}