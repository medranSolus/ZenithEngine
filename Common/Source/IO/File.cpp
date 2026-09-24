#include "IO/File.h"
#include <filesystem>

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
			U32 read = Utils::SafeCast<U32>(std::fread(buffer, 1, size, stdFile));
			if (read == size)
				return {};
			return EofResult::Make(read);
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
			U32 written = Utils::SafeCast<U32>(std::fwrite(buffer, 1, size, stdFile));
			if (written == size)
				return {};
			return EofResult::Make(written);
		}
		return platformImpl.Write(buffer, size);
	}

	Status File::Open(std::string_view fileName, FileFlags flags, U8** fileMapping) noexcept
	{
		ZE_ASSERT(!stdFile, "File already opened!");
		if (flags & FileFlag::CreateOnOpen)
		{
			U64 pos = fileName.find_last_of("\\/");
			if (pos != std::string_view::npos)
			{
				std::string_view dir = fileName.substr(0, pos);
				Status code = {};
				bool exists = std::filesystem::exists(dir, code);
				if (code)
				{
					ZE_CODE_WARNING(code, "Error checking if directory \"" + std::string(dir) + "\" exists, trying to create anyway.");
					exists = false;
				}
				if (!exists)
				{
					std::filesystem::create_directories(dir, code);
					if (code)
					{
						ZE_CODE_ERROR(code, "Error creating directory \"" + std::string(dir) + "\"!");
						return code;
					}
				}
			}
		}
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