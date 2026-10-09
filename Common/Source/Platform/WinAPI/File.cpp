#include "Platform/WinAPI/File.h"
#include "Platform/WinAPI/AsyncBackgroundThread.h"
#include "Platform/WinAPI/AsyncIoAwaiter.h"
#include <io.h>
#include <fcntl.h>

namespace ZE::Platform::WinAPI
{
	template<bool IS_READ, typename BuffBtr>
	Task<Status> File::PerformAsyncOperation(BuffBtr buffer, U32 size, U64 offset) noexcept
	{
		ZE_ASSERT(osFile, "File not opened!");
		if (buffer == nullptr || size == 0)
		{
			ZE_FAIL("Invalid file buffer!");

			Task<Status> task([]() noexcept -> Status { return std::make_error_code(std::errc::invalid_argument); });
			return task;
		}

		Task<Status> task([](HANDLE fileHandle, BuffBtr buffer, U32 requestedBytes, U64 offset) noexcept -> Task<Status>
			{
				Status stat = co_await AsyncIoAwaiter<IS_READ, BuffBtr>{ fileHandle, buffer, requestedBytes, offset };
				co_return stat;
			}, osFile, buffer, size, offset);
		task.Start();
		return task;
	}

	template<bool IS_READ, typename BuffBtr>
	Status File::PerformSyncOperation(BuffBtr buffer, U32 size, U64 offset) noexcept
	{
		ZE_ASSERT(osFile, "File not opened!");
		if (offset == UINT64_MAX)
			offset = currentOffset;

		Task<Status> task([](HANDLE fileHandle, BuffBtr buffer, U32 requestedBytes, U64 offset) noexcept -> Task<Status>
			{
				Status stat = co_await AsyncIoAwaiter<IS_READ, BuffBtr>{ fileHandle, buffer, requestedBytes, offset };
				co_return stat;
			}, osFile, buffer, size, offset);

		Status code = task.Get();
		offset += IO::EofResult::IsEOF(code) ? IO::EofResult::GetRealBytes(code) : size;
		currentOffset = offset;
		return code;
	}

	Expected<U64> File::GetSize(FILE* stdFile) const noexcept
	{
		HANDLE osHandle = osFile;
		if (!osHandle)
		{
			ZE_ASSERT(stdFile, "Without Windows handle, FILE pointer must be valid!");
			osHandle = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(stdFile)));
		}
		LARGE_INTEGER size = {};
		if (GetFileSizeEx(osHandle, &size))
			return static_cast<U64>(size.QuadPart);
		return std::unexpected(ZE_WIN_LAST_ERROR());
	}

	Status File::SetOffset(FILE* stdFile, U64 offset) noexcept
	{
		Status ret = {};
		if (stdFile)
		{
			if (_fseeki64_nolock(stdFile, static_cast<S64>(offset), SEEK_SET) != 0)
				ret = std::make_error_code(std::errc::invalid_argument);
		}
		else
		{
			ZE_ASSERT(osFile, "File not opened!");
			currentOffset = offset;
		}
		return ret;
	}

	U64 File::GetOffset(FILE* stdFile) const noexcept
	{
		if (stdFile)
			return static_cast<U64>(_ftelli64_nolock(stdFile));
		else
			return currentOffset;
	}

	Task<Status> File::ReadAsync(void* buffer, U32 size, U64 offset) noexcept
	{
		return PerformAsyncOperation<true>(buffer, size, offset);
	}

	Task<Status> File::WriteAsync(const void* buffer, U32 size, U64 offset) noexcept
	{
		return PerformAsyncOperation<false>(buffer, size, offset);
	}

	Status File::Read(void* buffer, U32 size, U64 offset) noexcept
	{
		return PerformSyncOperation<true>(buffer, size, offset);
	}

	Status File::Write(const void* buffer, U32 size, U64 offset) noexcept
	{
		return PerformSyncOperation<false>(buffer, size, offset);
	}

	Status File::Open(std::string_view fileName, IO::FileFlags flags, U8** fileMapping, FILE*& stdFile) noexcept
	{
		const bool write = flags & IO::FileFlag::WriteMode;
		const bool read = flags & IO::FileFlag::ReadMode || !write;
		const bool sequential = flags & IO::FileFlag::SequentialAccess;
		const bool random = flags & IO::FileFlag::RandomAccess;
		const bool create = flags & IO::FileFlag::CreateOnOpen;
		const bool truncate = flags & IO::FileFlag::TruncateOnOpen && write;
		const bool async = flags & IO::FileFlag::EnableAsync;

		// In case of long pathnames + normalization
		std::wstring osFilename = Utils::ToUTF16(fileName);
		if (osFilename.size() > 1 && osFilename.at(1) == L':')
		{
			osFilename = L"\\\\?\\" + osFilename;
			std::replace(osFilename.begin(), osFilename.end(), L'/', L'\\');
		}

		// First create OS file handle as it get's most of the options available
		osFile = CreateFileW(osFilename.c_str(), (write ? GENERIC_WRITE : 0) | (read ? GENERIC_READ : 0), !write ? 0 : FILE_SHARE_READ,
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
			return ZE_WIN_LAST_ERROR();
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
				return std::make_error_code(std::errc::bad_file_descriptor);
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
				return std::make_error_code(std::errc::bad_file_descriptor);
			}
		}
		else
		{
			// Enable usage of IOCP
			ZE_CODE_RET_FAILED(AsyncBackgroundThread::RegisterFileHandle(osFile));
		}

		if (fileMapping)
		{
			mapping = CreateFileMappingW(osFile, nullptr, write ? PAGE_READWRITE : PAGE_READONLY, 0, 0, nullptr);
			if (mapping == INVALID_HANDLE_VALUE)
			{
				mapping = nullptr;
				Close();
				return ZE_WIN_LAST_ERROR();
			}
			*fileMapping = reinterpret_cast<U8*>(MapViewOfFile(mapping, (write ? FILE_MAP_WRITE : FILE_MAP_READ), 0, 0, 0));
		}

		// stdFile now has ownership
		if (!async)
			osFile = nullptr;
		return {};
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