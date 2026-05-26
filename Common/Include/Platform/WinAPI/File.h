#pragma once
#include "IO/FileFlags.h"
#include "Error.h"
#include "Task.h"
#include "WinAPI.h"

namespace ZE::Platform::WinAPI
{
	class File final
	{
		HANDLE osFile = nullptr;
		HANDLE mapping = nullptr;
		mutable U64 currentOffset = 0; // Used only when async flag is set

		static void TransferCompletionCallback(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) noexcept;

		template<bool IS_READ, typename BuffBtr>
		Task<Status> PerformAsyncOperation(BuffBtr buffer, U32 size, U64 offset) noexcept;
		template<bool IS_READ, typename BuffBtr>
		Status PerformSyncOperation(BuffBtr buffer, U32 size) const noexcept;

	public:
		File() = default;
		ZE_CLASS_MOVE(File);
		~File() { Close(); }

		void SetOffset(FILE* stdFile, U64 offset) noexcept;

		Task<Status> ReadAsync(void* buffer, U32 size, U64 offset) noexcept { return PerformAsyncOperation<true>(buffer, size, offset); }
		Task<Status> WriteAsync(const void* buffer, U32 size, U64 offset) noexcept { return PerformAsyncOperation<false>(buffer, size, offset); }

		Status Read(void* buffer, U32 size) const noexcept;
		Status Write(const void* buffer, U32 size) const noexcept;

		Status Open(std::string_view fileName, IO::FileFlags flags, U8** fileMapping, FILE*& stdFile) noexcept;
		void Close(U8* fileMapping = nullptr) noexcept;
	};

#pragma region Functions
	template<bool IS_READ, typename BuffBtr>
	Task<Status> File::PerformAsyncOperation(BuffBtr buffer, U32 size, U64 offset) noexcept
	{
		ZE_ASSERT(osFile, "File not opened!");
		if (buffer == nullptr || size == 0)
		{
			ZE_FAIL("Invalid file buffer!");

			Task<Status> task(std::packaged_task<Status()>([]() noexcept -> Status { return std::make_error_code(std::errc::invalid_argument); }));
			return task;
		}

		std::unique_ptr<OVERLAPPED> overlapped = std::make_unique<OVERLAPPED>();
		overlapped->Offset = static_cast<U32>(offset & UINT32_MAX);
		overlapped->OffsetHigh = static_cast<U32>(offset >> 32);
		overlapped->hEvent = CreateEventW(nullptr, false, false, nullptr);

		BOOL operation;
		if constexpr (IS_READ)
			operation = ReadFileEx(osFile, buffer, size, overlapped.get(), File::TransferCompletionCallback);
		else
			operation = WriteFileEx(osFile, buffer, size, overlapped.get(), File::TransferCompletionCallback);

		if (operation == 0)
		{
			Status lastError = ZE_WIN_LAST_ERROR();
			[[maybe_unused]] const BOOL status = CloseHandle(overlapped->hEvent);
			ZE_ASSERT(status, "Error closing file event handle!");

			Task<Status> task(std::packaged_task<Status()>(std::bind([](Status code) noexcept -> Status { return code; }, lastError)));
			return task;
		}

		Task<Status> task(std::packaged_task<Status()>(std::bind([overlapped = std::move(overlapped)](U32 requestedBytes) noexcept -> Status
			{
				// Wait for async operation to complete
				Status code = {};
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
						else
							code = std::make_error_code(std::errc::io_error);
						wait = false;
						break;
					}
					case WAIT_IO_COMPLETION:
						break;
					default:
					{
						code = ZE_WIN_LAST_ERROR();
						wait = false;
						break;
					}
					}
				} while (wait);

				[[maybe_unused]] const BOOL status = CloseHandle(overlapped->hEvent);
				ZE_ASSERT(status, "Error closing file event handle!");

				return code;
			}, size)));
		return task;
	}
#pragma endregion
}