#pragma once
#include "Platform/WinAPI/DiskManager.h"
#include "GFX/Device.h"
ZE_WARNING_PUSH
#include "dstorage.h"
ZE_WARNING_POP

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

		WinAPI::DiskManager osDiskManager;

		DX::ComPtr<IStorageFactory> factory;
		DX::ComPtr<IStorageCustomDecompressionQueue> decompressQueue;
		DX::ComPtr<IStorageCompressionCodec> compressCodecGDeflate;

		DX::ComPtr<IStorageQueue> fileQueue;
		DX::ComPtr<IStorageQueue> memoryQueue;
		BoolAtom checkForDecompression = true;
		std::thread cpuDecompressionThread;

		static bool IsFileOnSSD(std::wstring_view path) noexcept;
		void DecompressAssets(Device& dev) const;

	public:
		DiskManager(GFX::Device& dev);
		ZE_CLASS_MOVE(DiskManager);
		~DiskManager();

		// IO API Internal

		IStorageFactory* GetFactory() const noexcept { return factory.Get(); }
	};
}