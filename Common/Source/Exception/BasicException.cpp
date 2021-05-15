#include "Exception/BasicException.h"

namespace ZE::Exception
{
	const char* BasicException::what() const noexcept
	{
		std::ostringstream stream;
		stream << "[Exception] " << GetType()
			<< ":\n[File] " << file << '@' << line;
		whatBuffer = stream.str();
		return whatBuffer.c_str();
	}
}