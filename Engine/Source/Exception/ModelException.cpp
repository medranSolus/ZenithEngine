#include "Exception/ModelException.h"

namespace ZE::Exception
{
	const char* ModelException::what() const noexcept
	{
		std::ostringstream stream;
		stream << BasicException::what()
			<< "\n[Assimp Error] " << error;
		whatBuffer = stream.str();
		return whatBuffer.c_str();
	}
}