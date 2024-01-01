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
		static constexpr U32 MAX_DECOMPRESSION_WAIT = 2000; // Number of miliseconds to wait for new decompression request to come
		// Custom decompression formats
		static constexpr DSTORAGE_COMPRESSION_FORMAT COMPRESSION_FORMAT_ZLIB = static_cast<DSTORAGE_COMPRESSION_FORMAT>(DSTORAGE_CUSTOM_COMPRESSION_0 + 1);

		WinAPI::DiskManager osDiskManager;

		DX::ComPtr<IStorageFactory> factory;
		DX::ComPtr<IStorageCustomDecompressionQueue> decompressQueue;
		DX::ComPtr<IStorageCompressionCodec> compressCodecGDeflate;

		DX::ComPtr<IStorageQueue> fileQueue;
		DX::ComPtr<IStorageQueue> memoryQueue;
		HANDLE fenceEvents[2] = { nullptr, nullptr };

		std::shared_mutex queueMutex;
		std::vector<EID> uploadQueue;
		std::vector<EID> submitQueue;
		std::vector<IResource*> uploadDestTextureQueue;
		std::vector<IResource*> submitDestTextureQueue;
		std::vector<std::shared_ptr<U8[]>> uploadSrcMemoryQueue;
		std::vector<std::shared_ptr<U8[]>> submitSrcMemoryQueue;

		BoolAtom checkForDecompression = true;
		std::jthread cpuDecompressionThread;

		static constexpr DSTORAGE_COMPRESSION_FORMAT GetCompressionFormat(IO::CompressionFormat compression) noexcept;
		static bool IsFileOnSSD(std::wstring_view path) noexcept;
		void DecompressAssets(Device& dev) const;
		void AddRequest(EID resourceID, std::shared_ptr<U8[]> src, IResource* texture) noexcept;

	public:
		DiskManager() = default;
		DiskManager(GFX::Device& dev);
		ZE_CLASS_MOVE(DiskManager);
		~DiskManager();

		constexpr bool IsGPUWorkPending() const noexcept { return submitDestTextureQueue.size(); }

		void StartUploadGPU(bool waitable) noexcept;
		bool WaitForUploadGPU(GFX::Device& dev, GFX::CommandList& cl);

		// IO API Internal

		IStorageFactory* GetFactory() const noexcept { return factory.Get(); }

		void AddFileBufferRequest(EID resourceID, IResource* dest, IO::File& file, U64 sourceOffset,
			U32 sourceBytes, IO::CompressionFormat compression, U32 uncompressedSize) noexcept;
		// Use srcStatic when data ref don't have to be taken, otherwise when life of buffer ends before finishing the upload, use srcCopy
		void AddMemoryBufferRequest(EID resourceID, IResource* dest, const void* srcStatic, std::shared_ptr<U8[]> srcCopy, U32 bytes) noexcept;
		void AddMemoryTextureRequest(EID resourceID, IResource* dest, std::shared_ptr<U8[]> src, U32 bytes) noexcept;
		void AddMemoryTextureArrayRequest(EID resourceID, IResource* dest, std::shared_ptr<U8[]> src,
			U32 bytes, U16 arrayIndex, U32 width, U32 height, bool lastElement) noexcept;
	};
}