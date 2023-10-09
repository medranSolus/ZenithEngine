#pragma once
#include "Platform/WinAPI/DiskManager.h"
#include "GFX/Device.h"
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

		BoolAtom checkForDecompression = true;
		std::jthread cpuDecompressionThread;

		static constexpr DSTORAGE_COMPRESSION_FORMAT GetCompressionFormat(IO::CompressionFormat compression) noexcept;
		static bool IsFileOnSSD(std::wstring_view path) noexcept;
		void DecompressAssets(Device& dev) const;

	public:
		DiskManager() = default;
		DiskManager(GFX::Device& dev);
		ZE_CLASS_MOVE(DiskManager);
		~DiskManager();

		void StartUploadGPU(bool waitable) noexcept;
		bool WaitForUploadGPU();

		// IO API Internal

		IStorageFactory* GetFactory() const noexcept { return factory.Get(); }

		void AddFileBufferRequest(EID resourceID, IO::File& file, IResource* dest, U64 sourceOffset,
			U32 sourceBytes, IO::CompressionFormat compression, U32 uncompressedSize);
		void AddMemoryBufferRequest(EID resourceID, IResource* dest, const void* src, U32 bytes);
	};
}