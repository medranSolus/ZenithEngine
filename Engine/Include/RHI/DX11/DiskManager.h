#pragma once
#include "GFX/CommandList.h"
#include "IO/CompressionFormat.h"
#include <latch>

namespace ZE::GFX
{
	class GFile;
}
namespace ZE::RHI::DX11
{
	class DiskManager final
	{
		U64 currentFenceValue = 0;
		U64 baseBucketFenceValue = 0;
		std::shared_mutex bucketMutex;
		std::deque<std::vector<Task<Status>>> statusBuckets;

		void MoveFrom(DiskManager&& disk) noexcept;

	public:
		DiskManager() = default;
		ZE_CLASS_NO_COPY(DiskManager);
		DiskManager(DiskManager&& disk) noexcept { MoveFrom(std::move(disk)); }
		DiskManager& operator=(DiskManager&& disk) noexcept { MoveFrom(std::move(disk)); return *this; }
		~DiskManager() = default;

		static Expected<DiskManager> Create(GFX::Device& dev) noexcept;

		constexpr void StartUploadGPU() const noexcept {}
		constexpr bool IsGPUWorkPending(DiskStatusHandle handle) const noexcept { return false; }

		Expected<DiskStatusHandle> SetGPUUploadWaitPoint() noexcept;
		Status WaitForUploadGPU(GFX::Device& dev, GFX::CommandList& cl, DiskStatusHandle handle) noexcept;

		// Gfx API Internal

		void AddFileBufferRequest(EID resourceID, DX::ComPtr<IResource> dest, GFX::GFile& file, U64 sourceOffset,
			U32 sourceBytes, IO::CompressionFormat compression, U32 uncompressedSize) noexcept;
		void AddFileTextureRequest(std::latch* barrier, DX::ComPtr<IResource> dest, GFX::GFile& file, U64 sourceOffset,
			U32 sourceBytes, IO::CompressionFormat compression, U32 uncompressedSize, U32 rowPitch, U32 depthPitch) noexcept;
		void AddTexturePackID(EID resourceID, std::unique_ptr<std::latch> barrier) noexcept;
	};
}