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

		reinterpret_cast<std::promise<U32>*>(lpOverlapped->hEvent)->set_value(dwErrorCode == 0 ? dwNumberOfBytesTransfered : 0);
	}

	template<bool IS_READ>
	bool File::PerformSyncOperation(void* buffer, U32 size, U64 offset) const noexcept
	{
		if (buffer == nullptr || size == 0)
		{
			ZE_FAIL("Invalid file buffer!");
			return false;
		}

		OVERLAPPED overlapped = {};
		overlapped.Offset = static_cast<U32>(offset & UINT32_MAX);
		overlapped.OffsetHigh = static_cast<U32>(offset >> 32);
		overlapped.hEvent = CreateEventW(nullptr, false, false, nullptr);

		BOOL operation;
		if constexpr (IS_READ)
			operation = ReadFileEx(file, buffer, size, &overlapped, File::SyncCompletionCallback);
		else
			operation = WriteFileEx(file, buffer, size, &overlapped, File::SyncCompletionCallback);

		bool success = false;
		if (operation != 0)
		{
			// Wait for async operation to complete
			if (WaitForSingleObject(overlapped.hEvent, INFINITE) == WAIT_OBJECT_0)
			{
				if (overlapped.Offset == 0 && size == overlapped.OffsetHigh)
					success = true;
			}
		}
		const BOOL status = CloseHandle(overlapped.hEvent);
		ZE_ASSERT(status, "Error closing file event handle!");
		return success;
	}

	template<bool IS_READ>
	std::future<U32> File::PerformAsyncOperation(void* buffer, U32 size, U64 offset) const noexcept
	{
		if (buffer == nullptr || size == 0)
		{
			ZE_FAIL("Invalid file buffer!");
			std::promise<U32> promise;
			promise.set_value(0);
			return promise.get_future();
		}

		OVERLAPPED overlapped = {};
		overlapped.Offset = static_cast<U32>(offset & UINT32_MAX);
		overlapped.OffsetHigh = static_cast<U32>(offset >> 32);
		overlapped.hEvent = new std::promise<U32>;

		BOOL operation;
		if constexpr (IS_READ)
			operation = ReadFileEx(file, buffer, size, &overlapped, File::AsyncCompletionCallback);
		else
			operation = WriteFileEx(file, buffer, size, &overlapped, File::AsyncCompletionCallback);

		if (operation != 0)
		{
			delete reinterpret_cast<std::promise<U32>*>(overlapped.hEvent);
			std::promise<U32> promise;
			promise.set_value(0);
			return promise.get_future();
		}
		return reinterpret_cast<std::promise<U32>*>(overlapped.hEvent)->get_future();
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
			const BOOL status = CloseHandle(file);
			file = nullptr;
			ZE_ASSERT(status, "Error closing file handle!");
		}
	}
}