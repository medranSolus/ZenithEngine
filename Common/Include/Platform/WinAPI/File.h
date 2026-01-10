#pragma once
#include "IO/FileFlags.h"
#include "Task.h"
#include "WinAPI.h"

namespace ZE::WinAPI
{
	class File final
	{
		HANDLE osFile = nullptr;
		HANDLE mapping = nullptr;
		mutable U64 currentOffset = 0; // Used only when async flag is set

		static void TransferCompletionCallback(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) noexcept;

		template<bool IS_READ, typename BuffBtr>
		Task<U32> PerformAsyncOperation(BuffBtr buffer, U32 size, U64 offset) noexcept;
		template<bool IS_READ, typename BuffBtr>
		bool PerformSyncOperation(BuffBtr buffer, U32 size) const noexcept;

	public:
		File() = default;
		ZE_CLASS_MOVE(File);
		~File() { Close(); }

		void SetOffset(FILE* stdFile, U64 offset) noexcept;

		Task<U32> ReadAsync(void* buffer, U32 size, U64 offset) noexcept { return PerformAsyncOperation<true>(buffer, size, offset); }
		Task<U32> WriteAsync(const void* buffer, U32 size, U64 offset) noexcept { return PerformAsyncOperation<false>(buffer, size, offset); }

		bool Read(void* buffer, U32 size) const noexcept;
		bool Write(const void* buffer, U32 size) const noexcept;

		bool Open(std::string_view fileName, IO::FileFlags flags, U8** fileMapping, FILE*& stdFile) noexcept;
		void Close(U8* fileMapping = nullptr) noexcept;
	};

#pragma region Functions
	template<bool IS_READ, typename BuffBtr>
	Task<U32> File::PerformAsyncOperation(BuffBtr buffer, U32 size, U64 offset) noexcept
	{
		ZE_ASSERT(osFile, "File not opened!");
		if (buffer == nullptr || size == 0)
		{
			ZE_FAIL("Invalid file buffer!");

			Task<U32> task(std::packaged_task<U32()>([]() -> U32 { return 0; }));
			return task;
		}

		OVERLAPPED* overlapped = new OVERLAPPED{};
		overlapped->Offset = static_cast<U32>(offset & UINT32_MAX);
		overlapped->OffsetHigh = static_cast<U32>(offset >> 32);
		overlapped->hEvent = CreateEventW(nullptr, false, false, nullptr);

		BOOL operation;
		if constexpr (IS_READ)
			operation = ReadFileEx(osFile, buffer, size, overlapped, File::TransferCompletionCallback);
		else
			operation = WriteFileEx(osFile, buffer, size, overlapped, File::TransferCompletionCallback);

		if (operation == 0)
		{
			[[maybe_unused]] const BOOL status = CloseHandle(overlapped->hEvent);
			ZE_ASSERT(status, "Error closing file event handle!");
			delete overlapped;

			Task<U32> task(std::packaged_task<U32()>([]() -> U32 { return 0; }));
			return task;
		}

		Task<U32> task(std::packaged_task<U32()>(std::bind([](OVERLAPPED* overlapped) -> U32
			{
				// Wait for async operation to complete
				U32 transferedBytes = 0;
				bool wait = true;
				do
				{
					switch (WaitForSingleObjectEx(overlapped->hEvent, INFINITE, TRUE))
					{
					case WAIT_OBJECT_0:
					{
						if (overlapped->Offset == 0)
							transferedBytes = overlapped->OffsetHigh;
						wait = false;
						break;
					}
					case WAIT_IO_COMPLETION:
						break;
					default:
						wait = false;
						break;
					}
				} while (wait);

				[[maybe_unused]] const BOOL status = CloseHandle(overlapped->hEvent);
				ZE_ASSERT(status, "Error closing file event handle!");
				delete overlapped;

				return transferedBytes;
			}, overlapped)));
		return task;
	}
#pragma endregion
}