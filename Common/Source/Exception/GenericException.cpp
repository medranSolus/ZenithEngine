#include "Exception/GenericException.h"

namespace ZE::Exception
{
	const char* GenericException::what() const noexcept
	{
		std::ostringstream stream;
		stream << BasicException::what()
			<< "\n[Info] " << GetInfo();
		whatBuffer = stream.str();
		return whatBuffer.c_str();
	}
}