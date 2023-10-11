#include "GFX/FfxException.h"

namespace ZE::GFX
{
	const char* FfxException::what() const noexcept
	{
		std::ostringstream stream;
		stream << GenericException::what()
			<< "\n[Code description] " << GetErrorString();
		whatBuffer = stream.str();
		return whatBuffer.c_str();
	}
}