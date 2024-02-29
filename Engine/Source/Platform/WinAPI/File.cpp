#include "Platform/WinAPI/File.h"

namespace ZE::WinAPI
{
	void File::SyncCompletionCallback(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) noexcept
	{
		ZE_ASSERT(lpOverlapped->hEvent != nullptr, "Empty file event handle!");

		// Store error code and bytes transfered in offset
		lpOverlapped->Offset = dwErrorCode;
		lpOverlapped->OffsetHigh = dwNumberOfBytesTransfered;
		SetEvent(lpOverlapped->hEvent);
	}

	void File::AsyncCompletionCallback(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) noexcept
	{
		ZE_ASSERT(lpOverlapped->hEvent != nullptr, "Empty file promise handle!");

		std::promise<U32>* promise = reinterpret_cast<std::promise<U32>*>(lpOverlapped->hEvent);
		promise->set_value(dwErrorCode == 0 ? dwNumberOfBytesTransfered : 0);
		delete promise;
	}

	bool File::Open(std::wstring_view fileName, IO::FileFlags flags) noexcept
	{
		const bool writeOnly = flags & IO::FileFlag::WriteOnly;
		ZE_ASSERT(writeOnly != static_cast<bool>(flags & IO::FileFlag::GpuReading)
			|| (flags & (IO::FileFlag::GpuReading | IO::FileFlag::WriteOnly)) == 0,
			"Cannot open file for reading by GPU and in write only mode at the same time!");
		if (writeOnly && (flags & IO::FileFlag::GpuReading))
			return false;

		// Consider FILE_FLAG_NO_BUFFERING
		file = CreateFileW(fileName.data(), writeOnly ? GENERIC_WRITE : GENERIC_READ, writeOnly ? 0 : FILE_SHARE_READ,
			nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, nullptr);
		if (file == INVALID_HANDLE_VALUE)
		{
			file = nullptr;
			return false;
		}
		return true;
	}

	void File::Close() noexcept
	{
		if (file)
		{
			[[maybe_unused]] const BOOL status = CloseHandle(file);
			file = nullptr;
			ZE_ASSERT(status, "Error closing file handle!");
		}
	}
}