#include "Platform/WinAPI/File.h"
#include <io.h>
#include <fcntl.h>

namespace ZE::WinAPI
{
	void File::TransferCompletionCallback(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) noexcept
	{
		ZE_ASSERT(lpOverlapped->hEvent != nullptr, "Empty file event handle!");

		// Store error code and bytes transfered in offset
		lpOverlapped->Offset = dwErrorCode;
		lpOverlapped->OffsetHigh = dwNumberOfBytesTransfered;
		SetEvent(lpOverlapped->hEvent);
	}

	template<bool IS_READ, typename BuffBtr>
	bool File::PerformSyncOperation(BuffBtr buffer, U32 size) const noexcept
	{
		OVERLAPPED overlapped = {};
		overlapped.Offset = static_cast<U32>(currentOffset & UINT32_MAX);
		overlapped.OffsetHigh = static_cast<U32>(currentOffset >> 32);
		overlapped.hEvent = CreateEventW(nullptr, false, false, nullptr);

		BOOL operation;
		if constexpr (IS_READ)
			operation = ReadFileEx(osFile, buffer, size, &overlapped, File::TransferCompletionCallback);
		else
			operation = WriteFileEx(osFile, buffer, size, &overlapped, File::TransferCompletionCallback);

		bool success = false;
		if (operation != 0)
		{
			// Wait for async operation to complete
			bool wait = true;
			do
			{
				switch (WaitForSingleObjectEx(overlapped.hEvent, INFINITE, TRUE))
				{
				case WAIT_OBJECT_0:
				{
					if (overlapped.Offset == 0 && size == overlapped.OffsetHigh)
					{
						success = true;
						currentOffset += size;
					}
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
		}
		[[maybe_unused]] const BOOL status = CloseHandle(overlapped.hEvent);
		ZE_ASSERT(status, "Error closing file event handle!");
		return success;
	}

	void File::SetOffset(FILE* stdFile, U64 offset) noexcept
	{
		if (stdFile)
		{
			const S32 status = _fseeki64_nolock(stdFile, static_cast<S64>(offset), SEEK_SET);
			ZE_ASSERT(status == 0, "Error setting file offset!");
		}
		else
		{
			ZE_ASSERT(osFile, "File not opened!");
			currentOffset = offset;
		}
	}

	bool File::Read(void* buffer, U32 size) const noexcept
	{
		ZE_ASSERT(osFile, "File not opened!");
		return PerformSyncOperation<true>(buffer, size);
	}

	bool File::Write(const void* buffer, U32 size) const noexcept
	{
		ZE_ASSERT(osFile, "File not opened!");
		return PerformSyncOperation<false>(buffer, size);
	}

	bool File::Open(std::string_view fileName, IO::FileFlags flags, U8** fileMapping, FILE*& stdFile) noexcept
	{
		const bool write = flags & IO::FileFlag::WriteMode;
		const bool read = flags & IO::FileFlag::ReadMode || !write;
		const bool sequential = flags & IO::FileFlag::SequentialAccess;
		const bool random = flags & IO::FileFlag::RandomAccess;
		const bool create = flags & IO::FileFlag::CreateOnOpen;
		const bool truncate = flags & IO::FileFlag::TruncateOnOpen && write;
		const bool async = flags & IO::FileFlag::EnableAsync;

		// First create OS file handle as it get's most of the options available
		osFile = CreateFileW(Utils::ToUTF16(fileName).data(), (write ? GENERIC_WRITE : 0) | (read ? GENERIC_READ : 0), !write ? 0 : FILE_SHARE_READ,
			nullptr,
			create ? (truncate ? CREATE_ALWAYS : OPEN_ALWAYS) : (truncate ? TRUNCATE_EXISTING : OPEN_EXISTING),
			(write ? FILE_ATTRIBUTE_NORMAL : FILE_ATTRIBUTE_READONLY)
			| (async ? FILE_FLAG_OVERLAPPED : 0)
			| (sequential ? FILE_FLAG_SEQUENTIAL_SCAN : 0)
			| (random ? FILE_FLAG_RANDOM_ACCESS : 0)
			| (flags & IO::FileFlag::WriteThrough ? FILE_FLAG_WRITE_THROUGH : 0),
			nullptr);

		if (osFile == INVALID_HANDLE_VALUE)
		{
			osFile = nullptr;
			return false;
		}

		if (!async)
		{
			// Translate OS handle to CRT file descriptor
			S32 fileDesc = _open_osfhandle(reinterpret_cast<intptr_t>(osFile),
				(write ? (read ? _O_RDWR : _O_WRONLY) : _O_RDONLY) | _O_BINARY
				| (sequential ? _O_SEQUENTIAL : 0)
				| (random ? _O_RANDOM : 0)
				| (create ? _O_CREAT : _O_EXCL)
				| (truncate ? _O_TRUNC : 0));
			if (fileDesc == -1)
			{
				Close();
				return false;
			}

			// Open file stream
			const char* mode = "";
			if (write)
			{
				if (read)
				{
					if (truncate)
						mode = "w+b";
					else if (create)
						mode = "a+b";
					else
						mode = "r+b";
				}
				else if (truncate)
					mode = "wb";
				else
					mode = "ab";
			}
			else
				mode = "rb";
			stdFile = _fdopen(fileDesc, mode);
			if (stdFile == nullptr)
			{
				[[maybe_unused]] const S32 status = _close(fileDesc);
				ZE_ASSERT(status, "Error closing CRT file handle!");
				osFile = nullptr;
				return false;
			}
		}

		if (fileMapping)
		{
			mapping = CreateFileMappingW(osFile, nullptr, write ? PAGE_READWRITE : PAGE_READONLY, 0, 0, nullptr);
			if (mapping == INVALID_HANDLE_VALUE)
			{
				mapping = nullptr;
				Close();
				return false;
			}
			*fileMapping = reinterpret_cast<U8*>(MapViewOfFile(mapping, (write ? FILE_MAP_WRITE : FILE_MAP_READ), 0, 0, 0));
		}

		// stdFile now has ownership
		if (!async)
			osFile = nullptr;
		return true;
	}

	void File::Close(U8* fileMapping) noexcept
	{
		if (mapping)
		{
			ZE_ASSERT(fileMapping, "Memory leak detected, need to unmap file first!");
			if (fileMapping)
			{
				[[maybe_unused]] const BOOL status = UnmapViewOfFile(fileMapping);
				ZE_ASSERT(status, "Error unmapping file!");
			}

			[[maybe_unused]] const BOOL status = CloseHandle(mapping);
			mapping = nullptr;
			ZE_ASSERT(status, "Error closing file mapping handle!");
		}
		if (osFile)
		{
			[[maybe_unused]] const BOOL status = CloseHandle(osFile);
			osFile = nullptr;
			ZE_ASSERT(status, "Error closing OS file handle!");
		}
		currentOffset = 0;
	}
}