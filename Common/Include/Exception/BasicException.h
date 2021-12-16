#pragma once
#include "Types.h"
#include <exception>
#include <string>

namespace ZE::Exception
{
	// Basic exception type, recomended virtual inheritance as every exception need to be derived from this class
	class BasicException : public std::exception
	{
		U32 line;
		std::string file;

	protected:
		mutable std::string whatBuffer;

	public:
		BasicException(U32 line, const char* file) noexcept : line(line), file(file) {}
		ZE_CLASS_DEFAULT(BasicException);
		virtual ~BasicException() = default;

		// Extract filename from filepath to hide full path in exceptions
		static constexpr const char* GetFilename(const char* path) noexcept;

		constexpr U32 GetLine() const noexcept { return line; }
		constexpr const std::string& GetFile() const noexcept { return file; }
		virtual constexpr const char* GetType() const noexcept { return "Basic Exception"; }

		virtual const char* what() const noexcept;
	};

	constexpr const char* BasicException::GetFilename(const char* path) noexcept
	{
		const char* file = path;
		const char* prevFile = path;
		while (*path)
		{
			++path;
			if (*path == '/' || *path == '\\')
			{
				if (file != prevFile)
					prevFile = file;
				file = path;
			}
		}
		return prevFile + 1;
	}
}

// Current file filename
#define __FILENAME__ ZE::Exception::BasicException::GetFilename(__FILE__)