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
		bool PerformSyncOperation(void* buffer, U32 size, U64 offset) const noexcept;
		template<bool IS_READ>
		std::future<U32> PerformAsyncOperation(void* buffer, U32 size, U64 offset) const noexcept;

	public:
		File() = default;
		ZE_CLASS_MOVE(File);
		~File() { Close(); }

		bool Open(IO::DiskManager& disk, std::string_view fileName, IO::FileFlags flags) noexcept { return Open(Utils::ToUTF16(fileName), flags); }
		void Close(IO::DiskManager& disk) noexcept { Close(); }

		bool Read(void* buffer, U32 size, U64 offset) const noexcept { return PerformSyncOperation<true>(buffer, size, offset); }
		bool Write(void* buffer, U32 size, U64 offset) const noexcept { return PerformSyncOperation<false>(buffer, size, offset); }

		std::future<U32> ReadAsync(void* buffer, U32 size, U64 offset) const noexcept { return PerformAsyncOperation<true>(buffer, size, offset); }
		std::future<U32> WriteAsync(void* buffer, U32 size, U64 offset) const noexcept { return PerformAsyncOperation<false>(buffer, size, offset); }

		// IO API Internal

		bool Open(std::wstring_view fileName, IO::FileFlags flags) noexcept;
		void Close() noexcept;
	};

#pragma region Functions
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

#pragma endregion
}