#include "DynamicCBuffer.h"

namespace GFX::Data::CBuffer
{
	void DynamicCBuffer::Copy(const DynamicCBuffer& buffer) noexcept(!IS_DEBUG)
	{
		assert(&GetRootElement() == &buffer.GetRootElement());
		std::copy(buffer.bytes.begin(), buffer.bytes.end(), bytes.begin());
	}
}