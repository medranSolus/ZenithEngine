#include "Exception/RenderGraphCompileException.h"

namespace ZE::Exception
{
	const char* RenderGraphCompileException::what() const noexcept
	{
		std::ostringstream stream;
		stream << BasicException::what()
			<< "\n[Message] " << GetMessage();
		whatBuffer = stream.str();
		return whatBuffer.c_str();
	}
}