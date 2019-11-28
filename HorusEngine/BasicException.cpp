#include "BasicException.h"

namespace Exception
{
	const char * BasicException::what() const throw()
	{
		std::ostringstream stream;
		stream << "[Exception] " << GetType()
			<< ":\n[File] " << file << '@' << line;
		whatBuffer = stream.str();
		return whatBuffer.c_str();
	}
}
