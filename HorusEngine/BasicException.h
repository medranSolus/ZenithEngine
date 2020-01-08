#pragma once
#include <exception>
#include <string>
#include <sstream>

namespace Exception
{
	// Basic exception type, recomended virtual inheritance as every exception need to be derived from this class
	class BasicException : public std::exception
	{
		unsigned line;
		std::string file;

	protected:
		mutable std::string whatBuffer;

	public:
		constexpr BasicException(unsigned int line, const char * file) noexcept : line(line), file(file) {}
		BasicException(const BasicException &) = default;
		BasicException & operator=(const BasicException &) = default;
		virtual ~BasicException() = default;

		constexpr unsigned int GetLine() const noexcept { return line; }
		constexpr const std::string & GetFile() const noexcept { return file; }
		inline virtual const char * GetType() const noexcept { return "Basic Exception"; }

		virtual const char * what() const throw();
	};
}
