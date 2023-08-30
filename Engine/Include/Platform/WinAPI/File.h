#pragma once
#include "IO/DiskManager.h"
#include "IO/FileFlags.h"

namespace ZE::WinAPI
{
	// File implementation for Windows
	class File final
	{
		HANDLE file = nullptr;

		static void SyncCompletionCallback(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) noexcept;
		static void AsyncCompletionCallback(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) noexcept;

		template<bool IS_READ>
		bool PerformSyncOperation(U8* buffer, U32 size, U64 offset) const noexcept;
		template<bool IS_READ>
		std::future<U32> PerformAsyncOperation(U8* buffer, U32 size, U64 offset) const noexcept;

	public:
		File() = default;
		ZE_CLASS_MOVE(File);
		~File() { Close(); }

		bool Open(IO::DiskManager& disk, std::string_view fileName, IO::FileFlags flags) noexcept { return Open(Utils::ToUTF16(fileName), flags); }
		void Close(IO::DiskManager& disk) noexcept { Close(); }

		bool Read(U8* buffer, U32 size, U64 offset) const noexcept { return PerformSyncOperation<true>(buffer, size, offset); }
		bool Write(U8* buffer, U32 size, U64 offset) const noexcept { return PerformSyncOperation<false>(buffer, size, offset); }

		std::future<U32> ReadAsync(U8* buffer, U32 size, U64 offset) const noexcept { return PerformAsyncOperation<true>(buffer, size, offset); }
		std::future<U32> WriteAsync(U8* buffer, U32 size, U64 offset) const noexcept { return PerformAsyncOperation<false>(buffer, size, offset); }

		// IO API Internal

		bool Open(std::wstring_view fileName, IO::FileFlags flags) noexcept;
		void Close() noexcept;
	};
}