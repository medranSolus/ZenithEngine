#include "Exception/ImageException.h"

namespace ZE::Exception
{
	const char* ImageException::what() const noexcept
	{
		std::ostringstream stream;
		stream << BasicException::what()
			<< "\n[Image Info] " << GetImageInfo();
		whatBuffer = stream.str();
		return whatBuffer.c_str();
	}
}