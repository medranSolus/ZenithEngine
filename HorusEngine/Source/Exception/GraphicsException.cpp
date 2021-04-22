#include "Exception/GraphicsException.h"

namespace Exception
{
	const char* GraphicsException::what() const noexcept
	{
		std::ostringstream stream;
		stream << WinApiException::what();
#ifdef _MODE_DEBUG
		stream << GetDebugInfo();
#endif
		whatBuffer = stream.str();
		return whatBuffer.c_str();
	}
}