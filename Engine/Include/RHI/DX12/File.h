#pragma once
#include "Platform/WinAPI/File.h"

namespace ZE::RHI::DX12
{
	class File final
	{
		WinAPI::File osFile;
		DX::ComPtr<IStorageFile> file;

	public:
		File() = default;
		ZE_CLASS_MOVE(File);
		~File() = default;

		void Close(IO::DiskManager& disk) noexcept { osFile.Close(); file = nullptr; }

		bool Read(void* buffer, U32 size, U64 offset) const noexcept { return osFile.Read(buffer, size, offset); }
		bool Write(void* buffer, U32 size, U64 offset) const noexcept { return osFile.Write(buffer, size, offset); }

		std::future<U32> ReadAsync(void* buffer, U32 size, U64 offset) const noexcept { return osFile.ReadAsync(buffer, size, offset); }
		std::future<U32> WriteAsync(void* buffer, U32 size, U64 offset) const noexcept { return osFile.WriteAsync(buffer, size, offset); }

		bool Open(IO::DiskManager& disk, std::string_view fileName, IO::FileFlags flags) noexcept;

		// IO API Internal

		IStorageFile* GetStorageFile() const noexcept { return file.Get(); }
	};
}