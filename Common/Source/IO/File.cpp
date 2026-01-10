#include "IO/File.h"

namespace ZE::IO
{
	bool File::Read(void* buffer, U32 size) const noexcept
	{
		if (buffer == nullptr || size == 0)
		{
			ZE_FAIL("Invalid file buffer!");
			return false;
		}

		if (stdFile)
			return std::fread(buffer, 1, size, stdFile) == size;
		return platformImpl.Read(buffer, size);
	}

	bool File::Write(const void* buffer, U32 size) const noexcept
	{
		if (buffer == nullptr || size == 0)
		{
			ZE_FAIL("Invalid file buffer!");
			return false;
		}

		if (stdFile)
			return std::fwrite(buffer, 1, size, stdFile) == size;
		return platformImpl.Write(buffer, size);
	}

	bool File::Open(std::string_view fileName, FileFlags flags, U8** fileMapping) noexcept
	{
		ZE_ASSERT(!stdFile, "File already opened!");
		return platformImpl.Open(fileName, flags, fileMapping, stdFile);
	}

	void File::Close(U8* fileMapping) noexcept
	{
		platformImpl.Close(fileMapping);
		if (stdFile)
		{
			[[maybe_unused]] const S32 status = std::fclose(stdFile);
			stdFile = nullptr;
			ZE_ASSERT(status == 0, "Error closing file handle!");
		}
	}
}