#pragma once
#include "Types.h"
#include <exception>
#include <string>

namespace Exception
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
		BasicException(BasicException&&) = default;
		BasicException(const BasicException&) = default;
		BasicException& operator=(BasicException&&) = default;
		BasicException& operator=(const BasicException&) = default;
		virtual ~BasicException() = default;

		constexpr U32 GetLine() const noexcept { return line; }
		constexpr const std::string& GetFile() const noexcept { return file; }
		virtual constexpr const char* GetType() const noexcept { return "Basic Exception"; }

		virtual const char* what() const noexcept;
	};
}