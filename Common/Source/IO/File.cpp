#include "IO/File.h"

namespace ZE::IO
{
	Status File::Read(void* buffer, U32 size) const noexcept
	{
		if (buffer == nullptr || size == 0)
		{
			ZE_FAIL("Invalid file buffer!");
			return std::make_error_code(std::errc::invalid_argument);
		}

		if (stdFile)
		{	
			if (std::fread(buffer, 1, size, stdFile) == size)
				return {};
			return std::make_error_code(std::io_errc::stream);
		}
		return platformImpl.Read(buffer, size);
	}

	Status File::Write(const void* buffer, U32 size) const noexcept
	{
		if (buffer == nullptr || size == 0)
		{
			ZE_FAIL("Invalid file buffer!");
			return std::make_error_code(std::errc::invalid_argument);
		}

		if (stdFile)
		{
			if (std::fwrite(buffer, 1, size, stdFile) == size)
				return {};
			return std::make_error_code(std::io_errc::stream);
		}
		return platformImpl.Write(buffer, size);
	}

	Status File::Open(std::string_view fileName, FileFlags flags, U8** fileMapping) noexcept
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