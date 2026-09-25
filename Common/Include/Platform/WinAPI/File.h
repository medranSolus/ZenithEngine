#pragma once
#include "IO/EofResult.h"
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
		U64 currentOffset = 0;

		template<bool IS_READ, typename BuffBtr>
		Task<Status> PerformAsyncOperation(BuffBtr buffer, U32 size, U64 offset) noexcept;
		template<bool IS_READ, typename BuffBtr>
		Status PerformSyncOperation(BuffBtr buffer, U32 size, U64 offset) noexcept;

	public:
		File() = default;
		ZE_CLASS_MOVE(File);
		~File() { Close(); }

		Expected<U64> GetSize(FILE* stdFile) const noexcept;
		Status SetOffset(FILE* stdFile, U64 offset) noexcept;
		U64 GetOffset(FILE* stdFile) const noexcept;

		Task<Status> ReadAsync(void* buffer, U32 size, U64 offset) noexcept;
		Task<Status> WriteAsync(const void* buffer, U32 size, U64 offset) noexcept;

		Status Read(void* buffer, U32 size, U64 offset) noexcept;
		Status Write(const void* buffer, U32 size, U64 offset) noexcept;

		Status Open(std::string_view fileName, IO::FileFlags flags, U8** fileMapping, FILE*& stdFile) noexcept;
		void Close(U8* fileMapping = nullptr) noexcept;
	};
}