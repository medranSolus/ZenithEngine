#pragma once
#include "Platform/WinAPI/DiskManager.h"
#include "GFX/CommandList.h"
#include "IO/CompressionFormat.h"
ZE_WARNING_PUSH
#include "dstorage.h"
ZE_WARNING_POP

namespace ZE::IO
{
	class File;
}
namespace ZE::RHI::DX12
{
	// Wrappers for DirectStorage interfaces (to avoid using multiple different versions)
	typedef IDStorageFactory                   IStorageFactory;
	typedef IDStorageCustomDecompressionQueue1 IStorageCustomDecompressionQueue;
	typedef IDStorageQueue2                    IStorageQueue;
	typedef IDStorageCompressionCodec          IStorageCompressionCodec;
	typedef IDStorageFile                      IStorageFile;

	class DiskManager final
	{
		// Number of miliseconds to wait for new decompression request to come
		static constexpr U32 MAX_DECOMPRESSION_WAIT = 2000;
		// Number of objects that can be processed in single query to DirectStorage at once
		static constexpr U32 MINIMAL_DECOPRESSED_OBJECTS_PER_TURN = 4;
		// Custom decompression formats
		static constexpr DSTORAGE_COMPRESSION_FORMAT DS_COMPRESSION_FORMAT_ZLIB = static_cast<DSTORAGE_COMPRESSION_FORMAT>(DSTORAGE_CUSTOM_COMPRESSION_0 + 1);

		enum class ResourceType : U8 { Buffer, Mesh, Texture, TextureCopySrc };

		struct UploadEntry
		{
			U64 CurrentFence;
			EID ResID;
			ResourceType Type;
			IResource* DestResource;
			std::shared_ptr<const U8[]> SrcMemory;
		};

		WinAPI::DiskManager osDiskManager;

		DX::ComPtr<IStorageFactory> factory;
		DX::ComPtr<IStorageCustomDecompressionQueue> decompressQueue;
		HANDLE decompressionEvent;
		DX::ComPtr<IStorageCompressionCodec> compressCodecGDeflate;

		DX::ComPtr<IStorageQueue> fileQueue;
		DX::ComPtr<IStorageQueue> memoryQueue;

		mutable std::shared_mutex queueMutex, fenceMutex;
		UA64 currentFenceValue = 0;
		std::vector<UploadEntry> uploadQueue;
		std::unordered_map<U64, std::array<HANDLE, 2>> fenceEvents;

		BoolAtom checkForDecompression = true;
		std::jthread cpuDecompressionThread;

		static constexpr DSTORAGE_COMPRESSION_FORMAT GetCompressionFormat(IO::CompressionFormat compression) noexcept;
		static bool IsFileOnSSD(std::wstring_view path) noexcept;
		void DecompressAssets(Device& dev) const;
		void AddRequest(EID resourceID, IResource* dest, ResourceType type, std::shared_ptr<const U8[]> src) noexcept;

	public:
		DiskManager() = default;
		DiskManager(GFX::Device& dev);
		ZE_CLASS_MOVE(DiskManager);
		~DiskManager();

		DiskStatusHandle SetGPUUploadWaitPoint() noexcept;
		void StartUploadGPU() noexcept;
		bool IsGPUWorkPending(DiskStatusHandle handle) const noexcept;
		bool WaitForUploadGPU(GFX::Device& dev, GFX::CommandList& cl, DiskStatusHandle handle);

		// IO API Internal

		IStorageFactory* GetFactory() const noexcept { return factory.Get(); }
		void AddTexturePackID(EID resourceID) noexcept { AddRequest(resourceID, nullptr, ResourceType::Texture, nullptr); }

		void AddFileBufferRequest(EID resourceID, IResource* dest, IO::File& file, U64 sourceOffset,
			U32 sourceBytes, IO::CompressionFormat compression, U32 uncompressedSize, bool isMesh) noexcept;
		// Use srcStatic when data ref don't have to be taken, otherwise when life of buffer ends before finishing the upload, use srcCopy
		void AddMemoryBufferRequest(EID resourceID, IResource* dest, const void* srcStatic, std::shared_ptr<const U8[]> srcCopy, U32 bytes, bool isMesh) noexcept;

		void AddFileTextureRequest(IResource* dest, IO::File& file, U64 sourceOffset,
			U32 sourceBytes, IO::CompressionFormat compression, U32 uncompressedSize, bool copySrc) noexcept;
		void AddMemoryTextureRequest(IResource* dest, std::shared_ptr<const U8[]> src, U32 bytes, bool copySrc) noexcept;
		void AddMemoryTextureArrayRequest(IResource* dest, std::shared_ptr<const U8[]> src,
			U32 bytes, U16 arrayIndex, U32 width, U32 height, bool lastElement, bool copySrc) noexcept;
	};
}